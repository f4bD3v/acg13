/*******************************************************************************
 *  variance.cpp
 *******************************************************************************
 *  Copyright (c) 2013 Alexandre Kaspar <alexandre.kaspar@a3.epfl.ch>
 *  For Advanced Computer Graphics, at the LGG / EPFL
 * 
 *        DO NOT REDISTRIBUTE
 ***********************************************/

#include <charts/qcustomplot.h>
#include <nori/bitmap.h>
#include <nori/common.h>
#include <nori/evaluator.h>
#include <nori/object.h>
#include <QDir>
#include <QString>
#include <QPixmap>
#include <QScopedPointer>
#include <QVector>
#include <iostream>

NORI_NAMESPACE_BEGIN

QColor convert(Color3f c, int alpha){
        // c *= 1.0f / 255.0f;
        return QColor(c.r(), c.g(), c.b(), alpha);
}

/**
 * Result evaluation by computing the histogram of 
 * the difference with a reference result which has 
 * been precomputed to a high precision.
 */
class Variance : public Evaluator {
public:
	Variance(const PropertyList &propList) {
                // reference for the variance computation (i.e. the expected image)
                m_referenceName = propList.getString("reference", "reference.exr");
                m_reference = new Bitmap(m_referenceName);
                // name of the output file
                m_output = propList.getString("output", "plot_%1.png");
                // size of the plot
                m_sizeX = propList.getInteger("sizeX", 600);
                m_sizeY = propList.getInteger("sizeY", 600);
                m_barRatio = propList.getColor("barRatio", Color3f(0.8f));
                m_semilogy = propList.getBoolean("semilogy", false);
                m_logBase = propList.getFloat("logbase", 10.0f);
                m_logMin = propList.getFloat("logmin", 1.0f / m_logBase);
                m_stacked = propList.getBoolean("stacked", true);
                m_split = propList.getBoolean("split", false);
                if(m_stacked && m_split){
                        std::cerr << "\nWarning: Stacked and split??? Disabling bar stacking since we split...\n";
                        m_stacked = false;
                }
                m_alphaPen = propList.getColor("alphaPen", Color3f(200));
                m_alphaFill = propList.getColor("alphaFill", Color3f(80));
                // plot colors
                m_penColor[0] = propList.getColor("redPen", Color3f(255.0, 98.0, 107.0));
                m_fillColor[0] = propList.getColor("redFill", Color3f(244.0, 200.0, 200.0));
                m_penColor[1] = propList.getColor("greenPen", Color3f(65.0, 195.0, 59.0));
                m_fillColor[1] = propList.getColor("greenFill", Color3f(170.0, 255.0, 170.0));
                m_penColor[2] = propList.getColor("bluePen", Color3f(101.0, 101.0, 247.0));
                m_fillColor[2] = propList.getColor("blueFill", Color3f(200.0, 200.0, 254.0));
                
                // type of histogram
                m_absDiff = propList.getBoolean("absDiff", false);
                // number of bins for the histogram
                m_cuts = propList.getInteger("cuts", 30);
                m_bins = 1 + (m_absDiff ? m_cuts : (2 * m_cuts) );
                m_cutSize = 1.0f / float(m_cuts);
	}

	virtual ~Variance() {
                if(m_reference) delete m_reference;
	}
        
        inline unsigned int binIdx(float delta) const {
                if(m_absDiff){
                        if(delta == 0.0) return 0;
                        return std::min(int(std::ceil(delta / m_cutSize)), int(m_cuts)); // i in [1;m_cuts]
                }else{
                        if(delta == 0.0) return m_cuts;
                        int f = delta > 0 ? 1 : -1; // side from zero bin
                        delta *= f; // remove its sign
                        int i = std::min(int(std::ceil(delta / m_cutSize)), int(m_cuts)); // i in [1;m_cuts]
                        int idx = m_cuts + f * i;
                        if(idx < 0) throw NoriException(QString("Negative index '%1' for %2").arg(idx).arg(delta));
                        return (unsigned int)idx;
                }
        }
        
        inline float binValue(unsigned int i) const {
                // std::cout << "i=" << i << "\n";
                if(m_absDiff)   return float(i * m_cutSize);
                else    return float(int(i) - int(m_cuts)) * m_cutSize; 
        }

	/// Evaluate the radiance
	void evaluate(const Bitmap *result) const {
                // we check that pictures are of the same size
                if(result->rows() != m_reference->rows() || result->cols() != m_reference->cols()){
                        std::cerr << "The reference picture has a different size! (curr=";
                        std::cerr << result->rows() << "x" << result->cols() << ", ref=";
                        std::cerr << m_reference->rows() << "x" << m_reference->cols() << ")\n";
                        return;
                }
                
                // image to produce
		QScopedPointer<QCustomPlot> plot(new QCustomPlot(NULL));
                plot->xAxis->setLabel("Deviation");
                plot->yAxis->setLabel("Instance count");
                if(m_semilogy){
                        plot->yAxis->setScaleType(QCPAxis::stLogarithmic);
                        plot->yAxis->setScaleLogBase(m_logBase);
                        plot->yAxis->setRangeLower(m_logMin);
                }
                // plot->setBaseSize(m_sizeX, m_sizeY);
                plot->resize(m_sizeX, m_sizeY);
                
                // variance plot
                typedef QCPBars* BarRef;
                BarRef bars[3];
                for(unsigned int i = 0; i < 3; ++i) bars[i] = new QCPBars(plot->xAxis, plot->yAxis);
                QVector<double> keys[3];
                for(unsigned int i = 0; i < 3; ++i) keys[i].resize(m_bins);
                for(unsigned int i = 0; i < m_bins; ++i){
                        keys[0][i] = keys[1][i] = keys[2][i] = binValue(i);
                }
                QVector<double> data[3];
                for(unsigned int i = 0; i < 3; ++i) data[i].resize(m_bins);
                
                // we compute the difference histogram
                // and the variance at the same time
                double var[3] = { 0.0, 0.0, 0.0 };
                const unsigned int N = result->rows() * result->cols();
                for(Bitmap::Index row = 0, rows = result->rows(); row < rows; ++row){
                        for(Bitmap::Index col = 0, cols = result->cols(); col < cols; ++col){
                                const Color3f &col_new = result->coeffRef(row, col);
                                const Color3f &col_ref = m_reference->coeffRef(row, col);
                                Color3f col_delta = col_ref - col_new;
                                for(unsigned int i = 0; i < 3; ++i){
                                        float d = col_delta.coeff(i);
                                        if(m_absDiff) d = std::abs(d);
                                        data[i][binIdx(d)] += 1; // one more in that bin
                                        // variance contribution
                                        var[i] += d * d;
                                }
                        }    
                }
                
                // get max for y scaling
                double maxY = 0, maxYStack = 0;
                for(unsigned int i = 0; i < m_bins; ++i){
                        maxY = std::max(maxY, data[0][i]);
                        maxY = std::max(maxY, data[1][i]);
                        maxY = std::max(maxY, data[2][i]);
                        maxYStack = std::max(maxYStack, data[0][i] + data[1][i] + data[2][i]);
                        // std::cout << "hist(#" << i << "=" << binValue(i);
                        // std::cout << ") = r:" << data[0][i] << ", g:" << data[1][i] << ", b:" << data[2][i] << "\n";
                }
                
                // styling
                QString names[3] = { "Red deviation", "Green deviation", "Blue deviation" };
                for(unsigned int i = 0; i < 3; ++i){
                        QCPBars *bar = bars[i];
                        if(m_stacked && i > 0) bar->moveAbove(bars[i - 1]); // for stacking
                        bar->setPen(QPen(convert(m_penColor[i], m_stacked ? 255 : m_alphaPen[i])));
                        bar->setBrush(QBrush(convert(m_fillColor[i], m_stacked ? 255 : m_alphaFill[i])));
                        bar->setData(keys[i], data[i]);
                        bar->setWidth(m_cutSize * m_barRatio[i]);
                        bar->setName(names[i]);
                        plot->addPlottable(bar);
                }
                
                /*
                g->setPen(QPen(Qt::red)); // edge color
                g->setBrush(QBrush(QColor(30, 40, 255, 150))); // fill color
                g->setLineStyle(QCPGraph::lsStepCenter); // type of data visu
                g->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, Qt::red, Qt::white, 7)); // point decoration
                g->setErrorType(QCPGraph::etNone); // error
                 */
                
                if(m_semilogy){
                        plot->yAxis->setRangeUpper(m_stacked ? maxYStack : maxY);
                        plot->xAxis->setRange((m_absDiff ? 0.0 : -1.0) - m_cutSize, 1.0 + m_cutSize);
                } else plot->rescaleAxes();
                plot->replot();
                
                // export plot
                if(m_output.length() > 0){
                        if(m_split){
                                QString ch_suffix[3] = { "red", "green", "blue" };
                                // one for each component
                                for(unsigned int i = 0; i < 3; ++i){
                                        plot->plottable(i)->setVisible(true);
                                        plot->plottable((i + 1) % 3)->setVisible(false);
                                        plot->plottable((i + 2) % 3)->setVisible(false);
                                        QString suffix = QString("%1_%2")
                                                .arg(NoriObjectFactory::version()).arg(ch_suffix[i]);
                                        QPixmap::grabWidget(plot.data()).save(absFileName(m_output.arg(suffix)));
                                }
                        } else {
                                QPixmap::grabWidget(plot.data()).save(absFileName(m_output.arg(NoriObjectFactory::version())));
                        }
                }
                
                // print out the result variance useful information (short)
                for(unsigned int i = 0; i < 3; ++i) var[i] /= N;
                std::cout << "\n\nVariance:\n\tRed  = " << var[0];
                std::cout << "\n\tGreen = " << var[1];
                std::cout << "\n\tBlue = " << var[2] << "\n";
                std::cout << "\nStddev:\n\tRed  = " << std::sqrt(var[0]);
                std::cout << "\n\tGreen = " << std::sqrt(var[1]);
                std::cout << "\n\tBlue = " << std::sqrt(var[2]) << "\n\n";
                
                // no need to delete the bars as they are managed by the plot
	}

	QString toString() const {
		return QString("Variance[ reference = %1 ]").arg(m_referenceName);
	}
        
private:
        // reference stuff
        Bitmap *m_reference;
	QString m_referenceName;
        // output stuff
        QString m_output;
        // data stuff
        unsigned int m_sizeX, m_sizeY;
        unsigned int m_cuts, m_bins;
        float m_cutSize;
        bool m_absDiff;
        bool m_split;
        // style stuff
        Color3f m_barRatio;
        Color3f m_penColor[3], m_fillColor[3];
        bool m_semilogy;
        float m_logBase, m_logMin;
        bool m_stacked;
        Color3f m_alphaPen, m_alphaFill;
};

NORI_REGISTER_CLASS(Variance, "variance");
NORI_NAMESPACE_END
