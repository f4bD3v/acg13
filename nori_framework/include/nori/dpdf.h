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

#if !defined(__DISCRETE_PDF_H)
#define __DISCRETE_PDF_H

#include <nori/common.h>

NORI_NAMESPACE_BEGIN

/**
 * \brief Discrete probability distribution
 * 
 * This data structure can be used to transform uniformly distributed
 * samples to a stored discrete probability distribution.
 * 
 * \ingroup libcore
 */
struct DiscretePDF {
public:
	/// Allocate memory for a distribution with the given number of entries
	explicit inline DiscretePDF(size_t nEntries = 0) {
		reserve(nEntries);
		clear();
	}

	/// Clear all entries
	inline void clear() {
		m_cdf.clear();
		m_cdf.push_back(0.0f);
		m_normalized = false;
	}

	/// Reserve memory for a certain number of entries
	inline void reserve(size_t nEntries) {
		m_cdf.reserve(nEntries+1);
	}

	/// Append an entry with the specified discrete probability
	inline void append(float pdfValue) {
		m_cdf.push_back(m_cdf[m_cdf.size()-1] + pdfValue);
	}

	/// Return the number of entries so far
	inline size_t size() const {
		return m_cdf.size()-1;
	}

	/// Access an entry by its index
	inline float operator[](size_t entry) const {
		return m_cdf[entry+1] - m_cdf[entry];
	}

	/// Have the probability densities been normalized?
	inline bool isNormalized() const {
		return m_normalized;
	}

	/**
	 * \brief Return the original (unnormalized) sum of all PDF entries
	 *
	 * This assumes that \ref normalize() has previously been called
	 */
	inline float getSum() const {
		return m_sum;
	}

	/**
	 * \brief Return the normalization factor (i.e. the inverse of \ref getSum())
	 *
	 * This assumes that \ref normalize() has previously been called
	 */
	inline float getNormalization() const {
		return m_normalization;
	}

	/**
	 * \brief Normalize the distribution
	 *
	 * \return Sum of the (previously unnormalized) entries
	 */
	inline float normalize() {
		m_sum = m_cdf[m_cdf.size()-1];
		if (m_sum > 0) {
			m_normalization = 1.0f / m_sum;
			for (size_t i=1; i<m_cdf.size(); ++i) 
				m_cdf[i] *= m_normalization;
			m_cdf[m_cdf.size()-1] = 1.0f;
			m_normalized = true;
		} else {
			m_normalization = 0.0f;
		}
		return m_sum;
	}

	/**
	 * \brief %Transform a uniformly distributed sample to the stored distribution
	 * 
	 * \param[in] sampleValue
	 *     An uniformly distributed sample on [0,1]
	 * \return
	 *     The discrete index associated with the sample
	 */
	inline size_t sample(float sampleValue) const {
		std::vector<float>::const_iterator entry = 
				std::lower_bound(m_cdf.begin(), m_cdf.end(), sampleValue);
		size_t index = (size_t) std::max((ptrdiff_t) 0, entry - m_cdf.begin() - 1);
		return std::min(index, m_cdf.size()-2);
	}

	/**
	 * \brief %Transform a uniformly distributed sample to the stored distribution
	 * 
	 * \param[in] sampleValue
	 *     An uniformly distributed sample on [0,1]
	 * \param[out] pdf
	 *     Probability value of the sample
	 * \return
	 *     The discrete index associated with the sample
	 */
	inline size_t sample(float sampleValue, float &pdf) const {
		size_t index = sample(sampleValue);
		pdf = operator[](index);
		return index;
	}

	/**
	 * \brief %Transform a uniformly distributed sample to the stored distribution
	 * 
	 * The original sample is value adjusted so that it can be "reused".
	 *
	 * \param[in, out] sampleValue
	 *     An uniformly distributed sample on [0,1]
	 * \return
	 *     The discrete index associated with the sample
	 */
	inline size_t sampleReuse(float &sampleValue) const {
		size_t index = sample(sampleValue);
		sampleValue = (sampleValue - m_cdf[index])
			/ (m_cdf[index + 1] - m_cdf[index]);
		return index;
	}

	/**
	 * \brief %Transform a uniformly distributed sample. 
	 * 
	 * The original sample is value adjusted so that it can be "reused".
	 *
	 * \param[in,out]
	 *     An uniformly distributed sample on [0,1]
	 * \param[out] pdf
	 *     Probability value of the sample
	 * \return
	 *     The discrete index associated with the sample
	 */
	inline size_t sampleReuse(float &sampleValue, float &pdf) const {
		size_t index = sample(sampleValue, pdf);
		sampleValue = (sampleValue - m_cdf[index])
			/ (m_cdf[index + 1] - m_cdf[index]);
		return index;
	}

	/**
	 * \brief Turn the underlying distribution into a
	 * human-readable string format
	 */
	QString toString() const {
		QString result = QString("DiscretePDF[sum=%1, "
			"normalized=%2, pdf = {").arg(m_sum).arg(m_normalized);

		for (size_t i=0; i<m_cdf.size(); ++i) {
			result += QString("%1").arg(operator[](i));
			if (i != m_cdf.size()-1)
				result += QString(", ");
		}
		return result + QString("}]");
	}
private:
	std::vector<float> m_cdf;
	float m_sum, m_normalization;
	bool m_normalized;
};

/// Alias sampling data structure (see \ref makeAliasTable() for details)
struct AliasEntry {
	/// Probability of sampling the current entry
	float prob;
	/// Index of the alias entry
	uint32_t index;
};

/**
 * \brief Create the lookup table needed for Walker's alias sampling
 * method implemented in \ref sampleAlias(). Runs in linear time.
 *
 * The basic idea of this method is that one can "redistribute" the 
 * probability mass of a distribution to make it uniform. Furthermore, 
 * this can be done in a way such that the probability of each entry in
 * the "flattened" PMF consists of probability mass from at most *two* 
 * entries in the original PMF. That then leads to an efficient O(1) 
 * sampling algorithm with a O(n) preprocessing step to set up this 
 * special decomposition.
 *
 * \return The original (un-normalized) sum of all probabilities 
 * in \c pdf.
 */
inline float makeAliasTable(
		AliasEntry *tbl, float *pdf, uint32_t k) {
	/* Allocate temporary storage for classification purposes */
	uint32_t *c = new uint32_t[k],
			 *c_short = c - 1, *c_long  = c + k;

	float sum = 0;
	for (size_t i=0; i<k; ++i)
		sum += pdf[i];

	float normalization = 1.0f / sum;
	for (uint32_t i=0; i<k; ++i) {
		/* For each entry, determine whether there is 
		   "too little" or "too much" probability mass */
		float value = k * normalization * pdf[i];
		if (value < 1)
			*++c_short = i;
		else if (value > 1)
			*--c_long  = i;
		tbl[i].prob  = value;
		tbl[i].index = i;
	}

	/* Perform pairwise exchanges while there are entries 
	   with too much probability mass */
	for (uint32_t i=0; i < k-1 && c_long - c < k; ++i) {
		uint32_t short_index = c[i],
		         long_index  = *c_long;

		tbl[short_index].index = long_index;
		tbl[long_index].prob  -= 1.0f - tbl[short_index].prob;

		if (tbl[long_index].prob <= 1.0f)
			++c_long;
	}

	delete[] c;

	return sum;
}

/// Generate a sample in constant time using the alias method
inline uint32_t sampleAlias(const AliasEntry *tbl, uint32_t k, float sample) {
	uint32_t l = std::min((uint32_t) (sample * k), k - 1);
	float prob = tbl[l].prob;

	sample = sample * k - l;

	if (prob == 1 || (prob != 0 && sample < prob))
		return l;
	else
		return tbl[l].index;
}

/**
 * \brief Generate a sample in constant time using the alias method
 *
 * This variation shifts and scales the uniform random sample so 
 * that it can be reused for another sampling operation
 */
inline uint32_t sampleAliasReuse(const AliasEntry *tbl, uint32_t k, float &sample) {
	uint32_t l = std::min((uint32_t) (sample * k), k - 1);
	float prob = tbl[l].prob;

	sample = sample * k - l;
	
	if (prob == 1 || (prob != 0 && sample < prob)) {
		sample /= prob;
		return l;
	} else {
		sample = (sample - prob) / (1 - prob);
		return tbl[l].index;
	}
}

NORI_NAMESPACE_END

#endif /* __DISCRETE_PDF_H */
