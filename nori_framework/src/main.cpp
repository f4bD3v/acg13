/*
    This file is part of Nori, a simple educational ray tracer

    Copyright (c) 2012 by Wenzel Jakob and Steve Marschner.

    Nori is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License Version 3
    as published by the Free Software Foundation.

    Nori is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <nori/parser.h>
#include <nori/scene.h>
#include <nori/camera.h>
#include <nori/block.h>
#include <nori/bitmap.h>
#include <nori/integrator.h>
#include <nori/gui.h>
#include <nori/designer.h>
#include <nori/object.h>
#include <boost/scoped_ptr.hpp>
#include <QApplication>
#include <string>
	
using namespace nori;

void render(Scene *scene, const QString &filename, int version) {
	const Camera *camera = scene->getCamera();
	Vector2i outputSize = camera->getOutputSize();

	/* Create a block generator (i.e. a work scheduler) */
	BlockGenerator blockGenerator(outputSize, NORI_BLOCK_SIZE);

	/* Allocate memory for the entire output image */
	ImageBlock result(outputSize, camera->getReconstructionFilter());
	result.clear();

	/* Launch the GUI */
	NoriWindow window(&result);

	/* Launch one render thread per core */
	int nCores = getCoreCount();
	std::vector<BlockRenderThread *> threads;
	for (int i=0; i<nCores; ++i) {
		BlockRenderThread *thread = new BlockRenderThread(
			scene, scene->getSampler(), &blockGenerator, &result);
		thread->start();
		threads.push_back(thread);
	}
		
	window.startRefresh();
	qApp->exec();
	window.stopRefresh();

	/* Wait for them to finish */
	for (int i=0; i<nCores; ++i) {
		threads[i]->wait();
		delete threads[i];
	}

	/* Now turn the rendered image block into 
	   a properly normalized bitmap */
	Bitmap *bitmap = result.toBitmap();
        
        /* Evaluate it if meaningful */
        const Evaluator *ev = scene->getEvaluator();
        if(ev) ev->evaluate(bitmap);

	/* Determine the filename of the output bitmap */
	QFileInfo inputInfo(filename);
	QString outputName = inputInfo.path() 
		+ QDir::separator() 
		+ inputInfo.completeBaseName() + (version < 0 ? QString(".exr") : QString("_%1.exr").arg(version));

	/* Save using the OpenEXR format */
	bitmap->save(outputName);

	delete bitmap;
}

int main(int argc, char **argv) {
	QApplication app(argc, argv);
	Q_INIT_RESOURCE(resources);

        if (argc != 2 && argc != 3) {
                cerr << "Syntax: nori <scene.xml>" << endl;
                return -1;
        }
        
        // hidden version number
        int version = -1;
        if (argc == 3) {
                version = atoi(argv[2]);
                if(version < 0){
                        cerr << "The version should be positive or null!\n";
                        return -2;
                }
                nori::NoriObjectFactory::setVersion((unsigned int)version);
                
                cout << "Using version " << version << "\n";
        }else   cout << "Using default version (0)\n";
        
        // the file name
        QString filename(argv[1]);
        
	try {
            if(filename == "--designer"){
                // designer mode
                Designer design;
                return qApp->exec();
                
            } else if (filename.endsWith(".exr")) {
                // image mode
                boost::scoped_ptr<Bitmap> bitmap(new Bitmap(filename));
                ImageBlock image(bitmap.get());

                /* Launch the GUI */
                NoriWindow window(&image);
                return qApp->exec();
                
            } else {
                // rendering mode
                boost::scoped_ptr<NoriObject> root(loadScene(filename));

		if (root->getClassType() == NoriObject::EScene) {
			/* The root object is a scene! Start rendering it.. */
			render(static_cast<Scene *>(root.get()), filename, version);
		}
            }
	} catch (const NoriException &ex) {
		cerr << "Caught a critical exception: " << qPrintable(ex.getReason()) << endl;
		return -1;
	}

	return 0;
}
