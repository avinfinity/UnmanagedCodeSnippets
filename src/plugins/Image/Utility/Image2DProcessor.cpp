//****************************************************************************
// (c) 2011 by the openOR Team
//****************************************************************************
// The contents of this file are available under the GPL v2.0 license
// or under the openOR comercial license. see
//   /Doc/openOR_free_license.txt or
//   /Doc/openOR_comercial_license.txt
// for Details.
//****************************************************************************
#define NOMINMAX
#include <algorithm>
#include <limits>
#include <cstdio>
#include <deque>

#include <openOR/Image/Image2DProcessor.hpp>
#include <openOR/Utility/main.hpp>

namespace openOR {
   namespace Image {

      Math::Vector2ui Image2DProcessor::NORMALIZE_8BIT = Math::create<Math::Vector2ui>(0, 255);
      Math::Vector2ui Image2DProcessor::NORMALIZE_16BIT = Math::create<Math::Vector2ui>(0, 65535);
      Math::Vector2ui Image2DProcessor::EMPTY_VECTOR = Math::create<Math::Vector2ui>(0, 0);
      std::map<Image2DProcessor::ImageType, std::string> Image2DProcessor::IMAGE_TYPE_ENDINGS;

      Image2DProcessor::Image2DProcessor() {
         if (IMAGE_TYPE_ENDINGS.size() == 0) {
            IMAGE_TYPE_ENDINGS.insert(std::pair<ImageType, std::string>(RAW, ".raw"));
            IMAGE_TYPE_ENDINGS.insert(std::pair<ImageType, std::string>(PNG16, ".png"));
            IMAGE_TYPE_ENDINGS.insert(std::pair<ImageType, std::string>(PNG8, ".png"));
         }
      }

      Image2DProcessor::~Image2DProcessor() {

      }

      std::tr1::shared_ptr<Image2DDataUI16> Image2DProcessor::adjustBrightness(std::tr1::shared_ptr<Image2DDataUI16> source,
         std::tr1::shared_ptr<Image2DDataUI16> preliminary) {

            std::tr1::shared_ptr<Image::Image2DDataUI16> tmp = createInstanceOf<Image::Image2DDataUI16>();
            std::tr1::shared_ptr<Image::Image2DDataUI16> retval = createInstanceOf<Image::Image2DDataUI16>();
            std::stringstream filename;

            int* histPreliminary = new int[256];
            int* histPreliminaryDiff = new int[255];
            int* histImage = new int[256];
            int* histImageDiff = new int[255];


            tmp->setSize(source->size());
            retval->setSize(source->size());

            // brightness correction - check for "preliminary" path
            if (preliminary == NULL) { 
               return source;
            } else if (Math::isEqualTo<Math::Vector2ui>(preliminary->size(), source->size())) {
               return source;
            }

            tmp = preliminary;
            preliminary = median(tmp, 3);
            tmp = preliminary;
            preliminary = normalize(tmp);

            // brightness correction - get histogramm of preliminary and image
            for (int h = 0; h < (int)source->size()(1); h++) {
               for (int w = 0; w < (int)source->size()(0); w++) {
                  if (histPreliminary[source->data()[h * source->size()(0) + w]] < 0) {
                     histPreliminary[source->data()[h * source->size()(0) + w]] = 0;
                  }
                  if (histImage[source->data()[h * source->size()(0) + w]] < 0) {
                     histImage[source->data()[h * source->size()(0) + w]] = 0;
                  }

                  histPreliminary[preliminary->data()[h * preliminary->size()(0) + w]]++;
                  histImage[source->data()[h * source->size()(0) + w]]++;
               }
            }

            // get derivative of histogram
            for (int n = 0; n < 255; n++) {
               if (histPreliminaryDiff[n] != 0) { continue; }

               // find next nonzero index
               int m = 0;
               for (m = n + 1; m < 255; m++) {
                  if (histPreliminary[m] != 0) { break; }
               }

               // get difference and set it until next nonzero index
               int diff = histPreliminary[m] - histPreliminary[n];
               for (int i = n; i < m; i++) {
                  histPreliminaryDiff[i] = diff;
               }
            }

            // analyse histograms to estimate light beam's min and max
            unsigned short min = 255;
            unsigned short max = 0;
            bool cond1 = false;
            for (short n = 0; n < 255; n++) {
               if (!cond1 && histPreliminary[n] < 1000 && histPreliminary[n] > 200 &&
                  histPreliminaryDiff[n] < 0) { 
                     cond1 = true;
                     std::cout << "condition 1 true at n = " << n << std::endl;
               }
               if (cond1 && histPreliminary[n] > 2000) { min = n; }
               if (min < n && histPreliminary[n] < 200 && histPreliminary[n] > 50) { max = n; }
            }
            if (max == 0) max = 255;

            // get negative of preliminary image
            for (int n = 0; n < (int)(source->size()(0) * source->size()(1)); n++) {
               preliminary->mutableData()[n] = std::max(0, 255 - preliminary->data()[n]);
               retval->mutableData()[n] = std::min(255, source->data()[n] + preliminary->data()[n]);
            }

            tmp = retval;
            retval = normalize(tmp);

            return retval;
      }


      std::vector<Image2DProcessor::Cluster> Image2DProcessor::findClusters(std::tr1::shared_ptr<Image::Image2DDataUI16> binary) {
         std::map<unsigned int, Math::Vector3ui > whites;
         std::map<unsigned int, Math::Vector3ui >::iterator itWhites;
         std::map<unsigned int, Math::Vector3ui >::iterator itWhitesFind;

         std::vector< Cluster > clusters;
         std::vector< Cluster >::iterator itClusters;
         std::deque< Math::Vector2ui > neighbours;
         Cluster cluster;
         std::vector< Math::Vector2ui >::iterator itCluster;
         int regionBorder = 2;

         // find white pixels and set them to "no region"
         for (int h = 0; h < (int)binary->size()(1); h++) {
            for (int w = 0; w < (int)binary->size()(0); w++) {
               if (binary->data()[h * binary->size()(0) + w] == 0) { continue; }

               whites.insert(std::pair<unsigned int, Math::Vector3ui>(h * binary->size()(0) + w, Math::create<Math::Vector3ui>(w, h, 0)));
            }
         }

         int incRegionId = 0;
         for (itWhites = whites.begin(); itWhites != whites.end(); itWhites++) {
            // continue if pixel is already assigned
            if ((*itWhites).second(2) != 0) { continue; }

            incRegionId++;
            neighbours.clear();
            cluster.pixels.clear();
            neighbours.push_back(Math::create<Math::Vector2ui>((*itWhites).second(0), (*itWhites).second(1)));

            // find neighbours of current white pixel
            while (neighbours.size() > 0) {
               // get and drop first elem (neighbour)
               Math::Vector2ui neighbour = neighbours.front();
               neighbours.pop_front();
               cluster.pixels.push_back(neighbour);

               if (cluster.boundMin(0) > neighbour(0)) { cluster.boundMin(0) = neighbour(0); }
               if (cluster.boundMin(1) > neighbour(1)) { cluster.boundMin(1) = neighbour(1); }
               if (cluster.boundMax(0) < neighbour(0)) { cluster.boundMax(0) = neighbour(0); }
               if (cluster.boundMax(1) < neighbour(1)) { cluster.boundMax(1) = neighbour(1); }

               // look in region if new white pixels are found
               int x = neighbour(0);
               int y = neighbour(1);
               for (int dy = -regionBorder; dy < regionBorder; dy++) {
                  for (int dx = -regionBorder; dx < regionBorder; dx++) {
                     if (dx == 0 && dy == 0) { continue; }
                     int w = std::min(std::max(x + dx, 0), static_cast<int>(binary->size()(0)));
                     int h = std::min(std::max(y + dy, 0), static_cast<int>(binary->size()(1)));

                     itWhitesFind = whites.find(h * binary->size()(0) + w);

                     if (itWhitesFind != whites.end()) {
                        if ((*itWhitesFind).second(2) == 0) {
                           // set its region
                           itWhitesFind->second(2) = incRegionId;

                           // insert as neighbours
                           neighbours.push_back(Math::create<Math::Vector2ui>(w, h));
                        }
                     }
                  }
               }

            }

            clusters.push_back(cluster);
         }

         return clusters;
      }


      std::tr1::shared_ptr<Image2DDataUI16> Image2DProcessor::subtract(std::tr1::shared_ptr<Image2DDataUI16> image) {
         std::tr1::shared_ptr<Image2DDataUI16> retval = createInstanceOf<Image2DDataUI16>();

         return retval;
      }

      std::tr1::shared_ptr<Image2DDataUI16> Image2DProcessor::getMask(std::tr1::shared_ptr<Image2DDataUI16> image) {
         uint nImageWidth = image->size()(0);
         uint nImageHeight = image->size()(1);

         std::tr1::shared_ptr<Image2DDataUI16> retval = createInstanceOf<Image2DDataUI16>();
         std::tr1::shared_ptr<Image2DDataUI16> image8 = normalize(image, NORMALIZE_8BIT, 25);

         cv::Mat cvImage = convert(image8);
         cv::Mat cvRetval(nImageWidth, nImageHeight, CV_8UC1);
         cv::Mat cvMask(nImageWidth, nImageHeight, CV_8UC1);

         // adaptive thresholding 8-bit image
         cv::adaptiveThreshold(cvImage, cvRetval, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY_INV, 51, 6);
         cvImage = cvRetval;

         // set border of xx pixels to black
         int m_bordersize = 25;
         for (uint y = 0; y < nImageHeight; y++) {
            for (uint x = 0; x < nImageWidth; x++) {
               if (((int)x > m_bordersize - 1 && (int)x < (int)nImageWidth - m_bordersize) &&
                  ((int)y > m_bordersize - 1 && (int)y < (int)nImageHeight - m_bordersize)) { continue; }
               cvImage.at<uint8>(x, y) = 0;
            }
         }

         // filter median
         cv::medianBlur(cvImage, cvRetval, 13);
         cvImage = cvRetval;

         // find clusters
         retval = convert(cvImage);
         std::vector<Cluster> c = findClusters(retval);

         // only keep ellipse cluster
         for (int n = 0; n < (int)c.size(); n++) {
            int dimX = c[n].boundMax(0) - c[n].boundMin(0);
            int dimY = c[n].boundMax(1) - c[n].boundMin(1);

            if ((dimX < 3 * dimY && dimY < 3 * dimX) &&
               (dimX > 30 && dimY > 30) &&
               (c[n].pixels.size() > 100)) { continue; }


            // paint all pixels black, if cluster does not fit
            for (int m = 0; m < (int)c[n].pixels.size(); m++) {
               int x = c[n].pixels[m](0);
               int y = c[n].pixels[m](1);

               cvImage.at<uint8>(y, x) = 0;
            }
         }

         // find ellipse points
         uint points = 4;
         float stepPointsX = (nImageWidth - 1) / points;
         float stepPointsY = (nImageHeight - 1) / points;
         int tmpX, tmpY;
         std::vector<int> posPointsX;
         std::vector<int> posPointsY;
         std::vector<cv::Point> posPoints;
         for (int n = 0; n <= (int)points; n++) {
            for (int m = 0; m <= (int)points; m++) {
               if ((n == 0 || n == points) || (m == 0 || m == points)) {
                  tmpX = -1, tmpY = -1;
                  uint x = n * stepPointsX;
                  uint y = m * stepPointsY;

                  findEllipseBorder(x, y, cvImage, tmpX, tmpY);
                  if (tmpX > -1 && tmpY > -1) {
                     posPointsX.push_back(tmpX);
                     posPointsY.push_back(tmpY);
                     cv::Point p = cvPoint(tmpY, tmpX);
                     posPoints.push_back(p);
                  }
               }
            }
         }


         if (posPoints.size() < 6) {
            std::tr1::shared_ptr<Image::Image2DDataUI16> imgNull = createInstanceOf<Image::Image2DDataUI16>();
            int max = nImageWidth * nImageHeight;
            imgNull->setSize(Math::create<Math::Vector2ui>(nImageWidth, nImageHeight), 0);
            return imgNull;
         }

         cv::Mat matPoints;
         cv::Mat(posPoints).convertTo(matPoints, CV_32F);
         cv::RotatedRect box = cv::fitEllipse(matPoints);

         // shrink ellipse
         box.size.width = box.size.width * 0.9f;
         box.size.height = box.size.height * 0.9f;

         cv::ellipse(cvMask, box, 255, -10);

         // apply mask on retval
         for (uint y = 0; y < nImageHeight; y++) {
            for (uint x = 0; x < nImageWidth; x++) {
               retval->mutableData()[y * nImageWidth + x] = (cvMask.at<uint8>(x, y) == 255) ? 0xFFFF : 0x0000;
            }
         }

         return retval;
      }

      void Image2DProcessor::flip(std::tr1::shared_ptr<Image2DDataUI16> image, bool horizontal, bool vertical) {
         if (!horizontal && !vertical) { return; }

         uint16 fTmp;

         if(horizontal) {
            for (uint y = 0; y < image->size()(1); ++y) {
               for (uint x = 0; x < image->size()(0) * 0.5; ++x) {
                  fTmp = image->data()[x + image->size()(0) * y];
                  image->mutableData()[x + image->size()(0) * y] = image->data()[image->size()(0) - x - 1 + image->size()(0) * y];
                  image->mutableData()[image->size()(0) - x - 1 + image->size()(0) * y] = fTmp;
               }
            }
         }

         if(vertical) {
            for (uint x = 0; x < image->size()(0); ++x) {      
               for (uint y = 0; y < image->size()(1) * 0.5; ++y) {
                  fTmp = image->data()[x + image->size()(0) * y];
                  image->mutableData()[x + image->size()(0) * y] = image->data()[x + image->size()(0) * (image->size()(1) - y - 1)];
                  image->mutableData()[x + image->size()(0) * (image->size()(1) - y - 1)] = fTmp;
               }
            }
         }
      }

      std::tr1::shared_ptr<Image2DDataUI16> Image2DProcessor::normalize(std::tr1::shared_ptr<Image2DDataUI16> image, const Math::Vector2ui& range, int ignoreDefectPixels, Math::Vector2ui& minmax) {
         uint16 imgMin, imgMax;
         float stretch = 1.0f;

         getMinMax(image, imgMin, imgMax, ignoreDefectPixels);
         minmax(0) = imgMin;
         minmax(1) = imgMax;

         stretch = (static_cast<float>(range(1)) - static_cast<float>(range(0))) / (static_cast<float>(imgMax) - static_cast<float>(imgMin));

         return normalize(image, stretch, static_cast<float>(imgMin));
      }

      std::tr1::shared_ptr<Image2DDataUI16> Image2DProcessor::normalize(std::tr1::shared_ptr<Image2DDataUI16> image, const float& stretch, const float& offset) {
         std::tr1::shared_ptr<Image2DDataUI16> retval = createInstanceOf<Image2DDataUI16>();
         retval->setSize(image->size());

         int maxI = image->size()(0) * image->size()(1);
         for (int n = 0; n < maxI; n++) {
            int val = static_cast<int>((image->data()[n] - offset) * stretch + 0.5f);
            retval->mutableData()[n] = static_cast<unsigned short>(std::max(std::min(val, 65535), 0));
         }

         return retval;
      }

      std::tr1::shared_ptr<Image2DDataUI16> Image2DProcessor::gauss(std::tr1::shared_ptr<Image2DDataUI16> image, const int& size) {
         std::tr1::shared_ptr<Image2DDataUI16> retval = createInstanceOf<Image2DDataUI16>();
         cv::Mat cvImage = convert(image);
         cv::Mat cvRetval;
         cv::Size szKernel(size, size);

         cv::GaussianBlur(cvImage, cvRetval, szKernel, 0, cv::BORDER_REFLECT);

         retval = convert(cvRetval);

         return retval;
      }

      std::tr1::shared_ptr<Image2DDataUI16> Image2DProcessor::median(std::tr1::shared_ptr<Image2DDataUI16> image, const int& size) {
         std::tr1::shared_ptr<Image2DDataUI16> retval = createInstanceOf<Image2DDataUI16>();
         cv::Mat cvImage = convert(image);
         cv::Mat cvRetval;

         cv::medianBlur(cvImage, cvRetval, size);

         retval = convert(cvRetval);

         return retval;
      }

      std::tr1::shared_ptr<Image2DDataUI16> Image2DProcessor::adaptiveThreshold(std::tr1::shared_ptr<Image2DDataUI16> image, const int& blockSize, const int& offset) {
         std::tr1::shared_ptr<Image2DDataUI16> retval = createInstanceOf<Image2DDataUI16>();
         cv::Mat cvImage = convert(image);
         cv::Mat cvRetval;

         cv::adaptiveThreshold(cvImage, cvRetval, 75, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY_INV, blockSize, offset);

         retval = convert(cvRetval);

         return retval;
      }

      std::tr1::shared_ptr<Image2DDataUI16> Image2DProcessor::erode(std::tr1::shared_ptr<Image2DDataUI16> image) {
         std::tr1::shared_ptr<Image2DDataUI16> retval = createInstanceOf<Image2DDataUI16>();
         cv::Mat cvImage = convert(image);
         cv::Mat cvRetval;
         cv::Mat cvOperator(3, 3, CV_8UC1, 1);

         cvOperator.at<uint8>(0, 0) = 0;
         cvOperator.at<uint8>(2, 2) = 0;
         cvOperator.at<uint8>(0, 2) = 0;
         cvOperator.at<uint8>(2, 0) = 0;

         cv::erode(cvImage, cvRetval, cvOperator);

         retval = convert(cvRetval);

         return retval;
      }

      std::tr1::shared_ptr<Image2DDataUI16> Image2DProcessor::dilate(std::tr1::shared_ptr<Image2DDataUI16> image) {
         std::tr1::shared_ptr<Image2DDataUI16> retval = createInstanceOf<Image2DDataUI16>();
         cv::Mat cvImage = convert(image);
         cv::Mat cvRetval;
         cv::Mat cvOperator(3, 3, CV_8UC1, 1);

         cvOperator.at<uint8>(0, 0) = 0;
         cvOperator.at<uint8>(2, 2) = 0;
         cvOperator.at<uint8>(0, 2) = 0;
         cvOperator.at<uint8>(2, 0) = 0;

         cv::dilate(cvImage, cvRetval, cvOperator);

         retval = convert(cvRetval);

         return retval;
      }

      void Image2DProcessor::drawCircle(std::tr1::shared_ptr<Image2DDataUI16>& image, const Math::Vector2f& center, const float& radius, const short& color, const int& thickness) {
         std::tr1::shared_ptr<Image2DDataUI16> retval = createInstanceOf<Image2DDataUI16>();
         cv::Mat cvImage = convert(image);
         cv::Point mid(static_cast<int>(center(1)), static_cast<int>(center(0)));
         int rad = static_cast<int>(radius);

         cv::circle(cvImage, mid, rad, cvScalar(color, color, color, 0), thickness, CV_AA, 0);

         image = convert(cvImage);
      }


      void Image2DProcessor::getMinMax(std::tr1::shared_ptr<Image2DDataUI16> image, unsigned short& min, unsigned short& max, int ignoreDefectPixels) {
         min = std::numeric_limits<unsigned short>::max();
         max = std::numeric_limits<unsigned short>::min();

         // get histogram of image
         unsigned short hist[65536] = {0};
         int maxI = image->size()(0) * image->size()(1);
         for (int i = 0; i < maxI; i++) {
            hist[image->data()[i]] += 1;
            min = std::min(min, image->data()[i]);
            max = std::max(max, image->data()[i]);
         }

         if (ignoreDefectPixels != 0) {
            int cntPxl = 0;
            for (int n = 65535; n != 0; n--) {
               cntPxl += hist[n];
               if (cntPxl > ignoreDefectPixels) {
                  max = n;
                  break;
               }
            }
         }
      }


      std::tr1::shared_ptr<Image2DDataUI16> Image2DProcessor::convert(const cv::Mat& source) {
         std::tr1::shared_ptr<Image2DDataUI16> retval = createInstanceOf<Image2DDataUI16>();
         Math::Vector2ui size;

         size(0) = source.rows;
         size(1) = source.cols;
         retval->setSize(size, 0);
         for (int y = 0; y < source.cols; y++) {
            for (int x = 0; x < source.rows; x++) {
               retval->mutableData()[y * source.rows + x] = source.at<uint8>(x, y);
            }
         }

         return retval;
      }

      cv::Mat Image2DProcessor::convert(std::tr1::shared_ptr<Image2DDataUI16> source) {
         cv::Mat retval(source->size()(0), source->size()(1), CV_8UC1);

         for (int y = 0; y < (int)source->size()(1); y++) {
            for (int x = 0; x < (int)source->size()(0); x++) {
               retval.at<uint8>(x, y) = source->data()[y * source->size()(0) + x];
            }
         }

         return retval;
      }

      void Image2DProcessor::findEllipseBorder(uint startX, uint startY, cv::Mat& binary, int& retX, int& retY) {
         float stepY = binary.rows * 0.5f - startY;
         float stepX = binary.cols * 0.5f - startX;

         // normalize
         float abs = std::sqrt(stepX * stepX + stepY * stepY);
         stepX /= abs;
         stepY /= abs;

         int steps = static_cast<int>(300.0f / std::max<float>(std::abs(stepX), std::abs(stepY)));

         for (int n = 1; n < steps; n++) {
            uint x = startX + n * stepX;
            uint y = startY + n * stepY;
            if (binary.at<uint8>(x, y) == 255) {
               retX = x;
               retY = y;
               return;
            }
            binary.at<uint8>(x, y) = 80;
         }
      }


      std::tr1::shared_ptr<Image::Image2DDataUI16> Image2DProcessor::loadImage(const std::string& filename) {
         std::tr1::shared_ptr<Image::Image2DDataUI16> pImg = createInstanceOf<Image::Image2DDataUI16>();
         QImage* qImg;

         // set size of pImg

         if (filename.find(".raw") != std::string::npos) {
            FILE* fin;
            //fopen_s(&fin, filename.c_str(), "rb");
            fin = fopen(filename.c_str(), "rb");
            if (fin == NULL) {
               // TODO: Image not found
               return std::tr1::shared_ptr<Image::Image2DDataUI16>();
            }
            pImg->setSize(Math::create<Math::Vector2ui>(1024, 1024));
            fread(pImg->mutableData(), sizeof(uint16), 1024 * 1024, fin);
            fclose(fin);

            //convertImage(pImg, m_loadImage);
         } else {
            qImg = new QImage(QString(filename.c_str()));
            if (qImg->isNull()) {
               // TODO: message("Image not found!");
               return std::tr1::shared_ptr<Image::Image2DDataUI16>();
            }
            qImg->convertToFormat(QImage::Format_RGB888);
            convertImage(qImg, pImg);
            //delete qImg;
         }

         return pImg;
      }


      void Image2DProcessor::saveImage(std::tr1::shared_ptr<Image::Image2DDataUI16> img, const std::string& filename, ImageType imgType, std::map<std::string, std::string> header) {
         if (Math::isEqualTo<Math::Vector2ui>(img->size(), Math::create<Math::Vector2ui>(0, 0))) { return; }

         // check filename for ending and set appropiate fileending depending on imagetype if necessary
         std::stringstream completeFilename;
         completeFilename << filename;
         if (filename.find(IMAGE_TYPE_ENDINGS[imgType]) == std::string::npos) {
            completeFilename << IMAGE_TYPE_ENDINGS[imgType];
         }

         if (imgType == RAW) {
            // file on the host computer for storing the image
            FILE *foutput;
            //fopen_s(&foutput, completeFilename.str().c_str(), "wb");
            foutput = fopen(completeFilename.str().c_str(), "wb"); 
            if (foutput == NULL) {
               std::cout << "Error opening image file to save file" << std::endl;
               return;
            }
            fwrite(img->data(), sizeof(uint16), img->size()(0) * img->size()(1), foutput);
            fclose(foutput);
            return;
         }

         QImage* qimg = NULL;
         convertImage(img, qimg, imgType);

         setImageHeader(qimg, header);

         if (qimg->isNull()) { std::cout << "converted image is null!" << std::endl; }
         if (!qimg->save(QString(completeFilename.str().c_str()), "PNG")) {
            std::cout << "saving " << completeFilename.str() << " failed :(" << std::endl;
         }
         delete qimg;
      }

      void Image2DProcessor::setImageHeader(QImage* qImg, const std::map<std::string, std::string> header) {
         std::map<std::string, std::string>::const_iterator it = header.begin();

         for (it; it != header.end(); it++) {
            qImg->setText(QString(it->first.c_str()), QString(it->second.c_str()));
         }
      }

      void Image2DProcessor::convertImage(std::tr1::shared_ptr<Image::Image2DDataUI16> pSource, QImage*& pTarget, ImageType imgType, bool mask) {
         pTarget = new QImage(pSource->size()(0), pSource->size()(1), QImage::Format_ARGB32);
         Image::Image2DDataUI16* pSrc = pSource.get();

         uchar* pPngData = pTarget->bits();
         if (pPngData == NULL) { return; }

         if (imgType == PNG16) {
            uint idxMax = pSource->size()(0) * pSource->size()(1);
            Image::Image2DProcessor processor;
            std::tr1::shared_ptr<Image::Image2DDataUI16> mask = processor.getMask(pSource);
            Image::Image2DDataUI16* pMask = mask.get();
            int maskFull = 0xFFFF;

            for(uint i = 0; i < idxMax; i++) {
               memcpy(&pPngData[i * 4 + 0], &pSrc->data()[i], sizeof(uint16));
               memcpy(&pPngData[i * 4 + 2], &pMask->data()[i], sizeof(uint16));
               //memcpy(&pPngData[i * 4 + 2], &maskFull, sizeof(uint16));
            }
         } else if (imgType == PNG8) {

            std::tr1::shared_ptr<Image::Image2DDataUI16> pTmp = createInstanceOf<Image::Image2DDataUI16>();
            pTmp->setSize(pSource->size());

            Image::Image2DDataUI16* pMask;
            std::tr1::shared_ptr<Image::Image2DDataUI16> mask;
            if (mask) {
               mask = getMask(pSource);
               pMask = mask.get();
            }

            pTmp = normalize(pSource, NORMALIZE_8BIT, 0);

            int nMaxIdx = pSource->size()(0) * pSource->size()(1);

            for (int n = 0; n < nMaxIdx; n++) {
               int idx = 4 * n;
               if (mask && pMask->data()[n] == 0) { pTmp->mutableData()[n] = 0; }
               pTarget->bits()[idx] = pTmp->data()[n];
               pTarget->bits()[idx + 1] = pTmp->data()[n];
               pTarget->bits()[idx + 2] = pTmp->data()[n];
               pTarget->bits()[idx + 3] = 0xFF;
            }
         }
      }

      void Image2DProcessor::convertImage(std::tr1::shared_ptr<Image::Image2DDataUI8> pSource, QImage*& pTarget) {
         pTarget = new QImage(pSource->size()(0), pSource->size()(1), QImage::Format_RGB888);
         Image::Image2DDataUI8* pSrc = pSource.get();

         uchar* pPngData = pTarget->bits();
         if (pPngData == NULL) { return; }

         int nMaxIdx = pSource->size()(0) * pSource->size()(1);

         for (int n = 0; n < nMaxIdx; n++) {
            int idx = 3 * n;
            pTarget->bits()[idx] = pSource->data()[n];
            pTarget->bits()[idx + 1] = pSource->data()[n];
            pTarget->bits()[idx + 2] = pSource->data()[n];
         }
      }

      void Image2DProcessor::convertImage(QImage* pSource, std::tr1::shared_ptr<Image::Image2DDataUI16> pTarget) {
         Math::Vector2ui sz;
         sz(0) = pSource->width();
         sz(1) = pSource->height();
         pTarget->setSize(sz, 0);

         int nIdx = 0;
         int nIdxEnd = (pSource->width() * pSource->height());

         if (pSource->text("16Bit").toFloat()) {
            for (int i = 0; i < nIdxEnd; i++) {
               pTarget->mutableData()[i] = (pSource->bits()[4 * i] << 8) | pSource->bits()[4 * i];
            }

         } else {
            int depth = pSource->depth();
            for (int i = 0; i < nIdxEnd; ++i) {
               nIdx = 4 * i;
               pTarget->mutableData()[i] = pSource->bits()[nIdx];
            }
         }
      }


   }
}
