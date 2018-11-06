#ifndef RANSAC_2D_H_
#define RANSAC_2D_H_

#include<vector>
#include<algorithm>
#include<set>
#include<map>
#include<limits.h>
#include<assert.h>
#include<iostream>

namespace sac
{
	enum modelType
	{
		MODEL_LINE2D,
		MODEL_CIRCLE2D,
		MODEL_ELLIPSE2D,
	};
}

typedef std::map<sac::modelType, unsigned int>::value_type sampleSizeModel;
const static sampleSizeModel sample_size_pairs[] = { sampleSizeModel(sac::MODEL_LINE2D, 2), 
sampleSizeModel(sac::MODEL_CIRCLE2D, 3), sampleSizeModel(sac::MODEL_ELLIPSE2D, 5) };

namespace sac
{
	const static std::map<sac::modelType, unsigned int> SAC_SAMPLE_SIZE(sample_size_pairs,
		sample_size_pairs + sizeof(sample_size_pairs) / sizeof(sampleSizeModel));
	
	class Point2D
	{
	public:
		Point2D(){ x = 0; y = 0; }
		~Point2D(){};
		double x, y;

		double calDistance(const Point2D pR)
		{
			return sqrt((x - pR.x)*(x - pR.x) + (y - pR.y)*(y - pR.y));
		}

		Point2D(double xx, double yy){ x = xx; y = yy; }
		Point2D(const Point2D &pR) { x = pR.x; y = pR.y; }

		Point2D operator + (Point2D p) { return Point2D(x + p.x, y + p.y); }
		Point2D operator - (Point2D p) { return Point2D(x - p.x, y - p.y); }
		double  operator * (Point2D p) { return (x*p.x + y*p.y); }
		Point2D operator / (Point2D p) { return Point2D(x / p.x, y / p.y); }
		Point2D operator * (double p) { return Point2D(x*p, y*p); }
		Point2D operator / (double p) { return Point2D(x / p, y / p); }
	};
	
	class ModelCoefficient
	{
	public:

		ModelCoefficient(double c0, double c1, double c2, double c3, double c4, double c5)
		{
			modelParam[0] = c0; modelParam[1] = c1; modelParam[2] = c2;
			modelParam[3] = c3; modelParam[4] = c4; modelParam[5] = c5;
		};

		ModelCoefficient(double c[6])
		{
			for (int i = 0; i < 6; i++)
				modelParam[i] = c[i];
		}

		ModelCoefficient()
		{
			for (int i = 0; i < 6; i++)
				modelParam[i] = 0;
		};

		ModelCoefficient(ModelCoefficient& mc)
		{
			for (int i = 0; i < 6; i++)
				modelParam[i] = mc.modelParam[i];
		};
		~ModelCoefficient(){};

		double modelParam[6];
	};
	
	class RansacModel
	{
	public:

		typedef std::vector<Point2D> PointCloud;

		RansacModel() :max_iterations_(500), iterations_(0), probability_(0.99), threshold_(0){};

		~RansacModel(){};

		RansacModel(const PointCloud &cloud, double threshold, int max_iterations)
			:max_iterations_(max_iterations),
			threshold_(threshold),
			iterations_(0),
			probability_(0.99)
		{
			setInputCloud(cloud);
		}

		RansacModel(const PointCloud &cloud, const std::vector<int>&indices, double threshold, int max_iterations)
			:input_(cloud),
			indices_(indices),
			max_iterations_(max_iterations),
			threshold_(threshold),
			iterations_(0),
			probability_(0.99)
		{
			if (indices_.size() > input_.size())
				indices_.clear();
			shuffled_indices_ = indices_;
		}

		inline void   setDistanceThreshold(double threshold){ threshold_ = threshold; }
		inline double getDistanceThreshold(){ return threshold_; }

		inline void setMaxIterations(int max_Iterations){ max_iterations_ = max_Iterations; }
		inline int  getMaxIteratios(){ return max_iterations_; }
		inline int  getRealIteratios(){ return iterations_; }

		virtual modelType getModelType() const = 0;

		inline int getSampleSize()
		{
			std::map<sac::modelType, unsigned int>::const_iterator it = SAC_SAMPLE_SIZE.find(getModelType());
			assert(it != SAC_SAMPLE_SIZE.end());
			return (it->second);
		}

		inline void getRandomSamples(std::vector<int> &indices, int nr_samples, std::set<int> &indice_subset)
		{
			indice_subset.clear();
			while ((int)indice_subset.size() < nr_samples)
				indice_subset.insert(indices[static_cast<int> (static_cast<double>(indices.size())*(rand() / (RAND_MAX + 1.0)))]);
		}

		inline void getModel(std::vector<int> &model){ model = model_; }

		inline void getInliers(std::vector<int> &inliers){ inliers = inliers_; }

		inline void getModelCoefficients(ModelCoefficient &model_coefficients){ model_coefficients = model_coefficients_; }

		inline std::vector<int> getIndices() const{ return indices_; }

		inline void setInputCloud(const PointCloud &cloud)
		{
			indices_.clear();

			input_ = cloud;
			if (indices_.empty())
			{
				indices_.resize(cloud.size());
				for (size_t i = 0; i < cloud.size(); i++)
					indices_[i] = static_cast<int>(i);
			}
			shuffled_indices_ = indices_;
		}
		
		inline void drawIndexSample(std::vector<int> &sample)
		{
			int sample_size = sample.size();
			int index_size = shuffled_indices_.size();
			sample.clear();
			sample.resize(sample_size);

			//for (int i = 0; i < sample_size; i++)
			//{
			//	srand(i * 1000 + iterations_ * 100);
			//	int iRand = rand();
			//	sample[i] = shuffled_indices_[iRand % (index_size - 1)];
			//}
			for (int i = 0; i < sample_size; i++)
			{
				srand(max_sample_checks_ * 10 + 100 * iterations_);
				//srand(100 * iterations_);
				int iRand = rand();
				std::swap(shuffled_indices_[i], shuffled_indices_[i + (iRand % (index_size - i))]);
			}
			std::copy(shuffled_indices_.begin(), shuffled_indices_.begin() + sample_size, sample.begin());

		}

		inline void getSamples(int &iterations, std::vector<int> &samples)
		{
			size_t sampleSize = getSampleSize();

			if (indices_.size()<sampleSize)
			{
				samples.clear();
				iterations = INT_MAX - 1;
				return;
			}

			samples.resize(sampleSize);
			for (int i = 0; i < max_sample_checks_; ++i)
			{
				drawIndexSample(samples);

				for (size_t j = 0; j < sampleSize; j++)
				{
					srand((i + 100)*(j + 20) + (iterations_ + 39) * 100);
					int iRand = rand();
					samples[j] = shuffled_indices_[iRand % (shuffled_indices_.size() - 1)];
				}

				if (isGoodSample(samples))
					return;
			}

			samples.clear();
		}

		inline void getVariance(double& disVariance)
		{
			double mean(0), stdev(0);
			for (size_t i = 0; i < error_sqr_dists_.size(); i++)
			{
				mean += error_sqr_dists_[i];
			}
			mean /= error_sqr_dists_.size();

			disVariance = mean;
		}
		
		inline void removeInliders(PointCloud& cloud, std::vector<int> inliers)
		{
			//remove inliers from cloud
			std::vector<int> remainPoints;
			PointCloud remainCloud;

			for (size_t i = 0; i < inliers.size(); i++)
			{
				if (i == 0)
				{
					for (int j = 0; j < inliers[0]; j++)
					{
						remainPoints.push_back(j);
					}
				}
				else if (i< inliers.size() - 1)
				{
					for (int j = inliers[i] + 1; j < inliers[i + 1]; j++)
					{
						remainPoints.push_back(j);
					}
				}
				else
				{
					for (size_t j = inliers[inliers.size() - 1] + 1; j < cloud.size(); j++)
					{
						remainPoints.push_back(j);
					}

				}
			}

			for (size_t i = 0; i < remainPoints.size(); i++)
			{
				remainCloud.push_back(cloud[remainPoints[i]]);
			}

			cloud.clear();
			for (size_t i = 0; i < remainCloud.size(); i++)
			{
				cloud.push_back(remainCloud[i]);
			}
			remainCloud.clear();
			remainPoints.clear();
		}
		
		
		//pure visual function to be implemented in son classes
		virtual bool isGoodSample(const std::vector<int> &samples) const = 0;

		virtual bool computeModelCoefficients(const std::vector<int> &samples, ModelCoefficient &model_coefficient) = 0;

		virtual int countWithinDistance(const ModelCoefficient model_coefficients, const double threshold) = 0;

		virtual void selectWithinDistance(const ModelCoefficient model_coefficients, const double threshold, std::vector<int> &inliers) = 0;

		virtual bool computeModel() = 0;
		

	protected:
		PointCloud input_;
		std::vector<int> indices_;
		std::vector<int> shuffled_indices_;
		int max_iterations_;
		int iterations_;
		double probability_;
		const int max_sample_checks_ = 1000;
		double threshold_;
		std::vector<int> model_;

		ModelCoefficient model_coefficients_;

		std::vector<int> inliers_;
		std::vector<double> error_sqr_dists_;

	};

}

#endif
