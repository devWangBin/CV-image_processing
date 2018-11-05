#ifndef RANSAC_CIRCLE2D_CPP_
#define RANSAC_CIRCLE2D_CPP_

#include "ransac_circle2d.h"

namespace sac
{

	bool ransacModelCircle2D::isGoodSample(const std::vector<int>& samples) const
	{
		Point2D p0 = input_[samples[0]], p1 = input_[samples[1]], p2 = input_[samples[2]];

		Point2D d01 = p1 - p0;
		Point2D d02 = p2 - p0;

		Point2D d = d01 / d02;
		return (abs(d.x - d.y) > 0.001);
	}

	bool ransacModelCircle2D::computeModelCoefficients(const std::vector<int> &samples, ModelCoefficient &model_coefficient)
	{
		if (samples.size() != 3)
			return false;

		Point2D p1 = input_[samples[0]], p2 = input_[samples[1]], p3 = input_[samples[2]];

		double a = 2 * (p2.x - p1.x), b = 2 * (p2.y - p1.y), c = p2.x*p2.x + p2.y*p2.y - p1.x*p1.x - p1.y*p1.y;
		double d = 2 * (p3.x - p2.x), e = 2 * (p3.y - p2.y), f = p3.x*p3.x + p3.y*p3.y - p2.x*p2.x - p2.y*p2.y;

		double x = (b*f - e*c) / (b*d - e*a);
		double y = (d*c - a*f) / (b*d - e*a);
		double r = sqrt((x - p1.x)*(x - p1.x) + (y - p1.y)*(y - p1.y));


		// radius of target circle has already been set
		if (m_dSpecificRadius > 0)
		{
			double cDiffRatio = abs(m_dSpecificRadius - r) / m_dSpecificRadius;
			if (cDiffRatio > m_dDiffRatio)
				return false;
		}

		if (m_vMultiRadius.size() != 0)
		{
			bool isIn = true;
			for (size_t i = 0; i < m_vMultiRadius.size(); i++)
			{
				double cDiffRatio = abs(m_vMultiRadius[i] - r) / m_vMultiRadius[i];				
				isIn = isIn && (cDiffRatio > m_dDiffRatio);
			}

			if (isIn)
				return false;
		}

		model_coefficient.modelParam[0] = x;
		model_coefficient.modelParam[1] = y;
		model_coefficient.modelParam[2] = r;

		return true;
	}

	int ransacModelCircle2D::countWithinDistance(const ModelCoefficient model_coefficients, const double threshold)
	{
		Point2D cP(model_coefficients.modelParam[0], model_coefficients.modelParam[1]);
		double radius = model_coefficients.modelParam[2];

		int count(0);
		for (size_t i = 0; i < indices_.size(); i++)
		{
			Point2D iP = input_[indices_[i]];
			double distance = iP.calDistance(cP);

			if (abs(distance - radius) < threshold)
				count++;
		}

		return count;
	}

	void ransacModelCircle2D::selectWithinDistance(const ModelCoefficient model_coefficients, const double threshold, std::vector<int> &inliers)
	{
		Point2D cP(model_coefficients.modelParam[0], model_coefficients.modelParam[1]);
		double radius = model_coefficients.modelParam[2];

		inliers.resize(indices_.size());
		error_sqr_dists_.resize(indices_.size());

		int count(0);
		for (size_t i = 0; i < indices_.size(); i++)
		{
			Point2D iP = input_[indices_[i]];
			double distance = iP.calDistance(cP);

			if (abs(distance - radius) < threshold)
			{
				inliers[count] = indices_[i];
				error_sqr_dists_[count] = abs(distance - radius);
				count++;
			}
		}

		inliers.resize(count);
		error_sqr_dists_.resize(count);
	}

	bool ransacModelCircle2D::computeModel()
	{
		//warn and exit if no threshold was set	
		assert(threshold_ != std::numeric_limits<double>::max());

		iterations_ = 0;
		int n_best_inliers_count = -INT_MAX;
		double log_probability = log(1.0 - probability_);
		double one_over_indices = 1 / static_cast<double>(getIndices().size());

		int n_inliers_count(0);
		int skipped_count = 0;
		const int max_skip = max_iterations_ * 10;

		ModelCoefficient model_coeff;
		std::vector<int> selection;

		while (iterations_ < max_iterations_ && skipped_count < max_skip)
		{
			getSamples(iterations_, selection);
			assert(selection.size() != 0);

			if (!computeModelCoefficients(selection, model_coeff))
			{
				++skipped_count;
				iterations_++;
				continue;
			}

			n_inliers_count = countWithinDistance(model_coeff, threshold_);
			if (n_inliers_count > n_best_inliers_count)
			{
				n_best_inliers_count = n_inliers_count;
				model_ = selection;
				model_coefficients_ = model_coeff;

				//compute the k parameter
				//TODO

			}

			iterations_++;
			if (iterations_ > max_iterations_)
				break;
		}

		if (model_.size() == 0)
		{
			inliers_.clear();
			return false;
		}

		selectWithinDistance(model_coefficients_, threshold_, inliers_);
		return true;
	}

}
#endif
