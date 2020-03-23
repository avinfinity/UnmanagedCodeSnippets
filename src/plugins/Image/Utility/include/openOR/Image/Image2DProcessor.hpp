//****************************************************************************
// (c) 2011 by the openOR Team
//****************************************************************************
// The contents of this file are available under the GPL v2.0 license
// or under the openOR comercial license. see
//   /Doc/openOR_free_license.txt or
//   /Doc/openOR_comercial_license.txt
// for Details.
//****************************************************************************

#ifndef openOR_Image_Image2DProcessor_hpp
#define openOR_Image_Image2DProcessor_hpp

#include <openOR/Image/Image2DData.hpp>
#include <openOR/Defs/Image_Utility.hpp>
#include <openOR/Math/math.hpp>

#include <QImage>

#ifdef __linux__
 #include <cv.h>
 #include <cxcore.h>
#else
 #include <cv.hpp>
#endif

//#include <highgui.h>


#include <map>

namespace openOR {
    namespace Image {

        struct OPENOR_IMAGE_UTILITY_API Image2DProcessor {

        public:

            struct OPENOR_IMAGE_UTILITY_API Cluster {
                std::vector<Math::Vector2ui> pixels;
                Math::Vector2ui boundMax;
                Math::Vector2ui boundMin;
            };

            typedef enum {
                RAW, PNG16, PNG8
            } ImageType;

            Image2DProcessor();
            ~Image2DProcessor();

            std::tr1::shared_ptr<Image2DDataUI16> adjustBrightness(std::tr1::shared_ptr<Image2DDataUI16> source, std::tr1::shared_ptr<Image2DDataUI16> preliminary);
            std::tr1::shared_ptr<Image2DDataUI16> getMask(std::tr1::shared_ptr<Image2DDataUI16> image);
            void flip(std::tr1::shared_ptr<Image2DDataUI16> image, bool horizontal = false, bool vertical = false);

            std::tr1::shared_ptr<Image2DDataUI16> subtract(std::tr1::shared_ptr<Image2DDataUI16> image);
            std::tr1::shared_ptr<Image2DDataUI16> normalize(std::tr1::shared_ptr<Image2DDataUI16> image, const Math::Vector2ui& range = NORMALIZE_8BIT, int ignoreDefectPixels = 0, Math::Vector2ui& minmax = EMPTY_VECTOR);
            std::tr1::shared_ptr<Image2DDataUI16> normalize(std::tr1::shared_ptr<Image2DDataUI16> image, const float& stretch, const float& offset = 0.0f);
            std::tr1::shared_ptr<Image2DDataUI16> gauss(std::tr1::shared_ptr<Image2DDataUI16> image, const int& size);
            std::tr1::shared_ptr<Image2DDataUI16> median(std::tr1::shared_ptr<Image2DDataUI16> image, const int& size);
            std::tr1::shared_ptr<Image2DDataUI16> erode(std::tr1::shared_ptr<Image2DDataUI16> image);
            std::tr1::shared_ptr<Image2DDataUI16> dilate(std::tr1::shared_ptr<Image2DDataUI16> image);

            std::tr1::shared_ptr<Image2DDataUI16> adaptiveThreshold(std::tr1::shared_ptr<Image2DDataUI16> image, const int& blockSize, const int& offset);

            std::vector<Cluster> findClusters(std::tr1::shared_ptr<Image2DDataUI16> binary);

            void drawCircle(std::tr1::shared_ptr<Image2DDataUI16>& image, const Math::Vector2f& center, const float& radius, const short& color, const int& thickness);

            //! \brief Loads an image from file.
            //!
            //! Loads an image from the given filename and returns it.
            //! \param[in] filename Path to the image file.
            //! \param[in] broadcast (optional) Indicates if the loaded image is broadcasted, \c false by default.
            //! \return Image content loaded from file.
            std::tr1::shared_ptr<Image::Image2DDataUI16> loadImage(const std::string& filename);

                        //! \brief Saves the given image and does some additional operations if applicable.
            //!
            //! Saves the passed image in the specified file. Writes the (optionally) passed environmental parameters
            //! to the header of the image. If a bag of detected markers is passed, they are painted into the image.
            //! \param[in] image Content of the image (16 bit).
            //! \param[in] filename Path to the file the image should be saved to.
            //! \param[in] is8bit (optional) Indicates whether the image should be save 8 bit or 16 bit (default).
            //! \param[in] params (optional) Parameters that describe the environment in which the image was taken, zero
            //!					  filled matrix by default. If the argument is not passed, the function has a look, if
            //!					  parameters were already calculated for this queue position and takes them instead.
            //! \param[in] markers (optional) Bag of detected markers that should be painted in the image.
            void saveImage(std::tr1::shared_ptr<Image::Image2DDataUI16> image, const std::string& filename, ImageType imgType = RAW, std::map<std::string, std::string> header = std::map<std::string, std::string>());

            //! \brief Converts an openOR::Image::Image2DDataUI16 into a QImage.
            //!
            //! Copies the data of an openOR::Image::Image2DDataUI16 to a QImage. The data can be stored either in 2
            //! channels of 16-bit, where the first channel contains the luminance data and the second alpha
            //! blending information. Or in 4 channels of 8-bit, commonly RGBA. The format of the QImage is RGBA32 in
            //! both cases.
            //! \param[in] pSource Shared pointer to the souce image.
            //! \param[out] pTarget Pointer to target QImage.
            //! \param[in] imgType Enum, that indicates whether pTarget should contain 8- or 16-bit data.
            void convertImage(std::tr1::shared_ptr<Image::Image2DDataUI16> pSource, QImage*& pTarget, ImageType imgType = PNG8, bool mask = true);

            //! \brief Converts a QImage into an openOR::Image::Image2DDataUI16
            //!
            //! Copies the data of a QImage into an openOR::Image::Image2DDataUI16.
            //! \todo Must recognize if 8-bit or 16-bit data are represented
            //! \param[in] pSource Pointer to source image.
            //! \param[out] pTarget Shared pointer to target image.
            void convertImage(QImage* pSource, std::tr1::shared_ptr<Image::Image2DDataUI16> pTarget);
			void convertImage(std::tr1::shared_ptr<Image::Image2DDataUI8> pSource, QImage*& pTarget);
            static Math::Vector2ui NORMALIZE_8BIT;
            static Math::Vector2ui NORMALIZE_16BIT;
            static Math::Vector2ui EMPTY_VECTOR;
            static std::map<ImageType, std::string> IMAGE_TYPE_ENDINGS;

        private:
            void getMinMax(std::tr1::shared_ptr<Image2DDataUI16> image, unsigned short& min, unsigned short& max, int ignoreDefectPixels = 0);
            void findEllipseBorder(uint startX, uint startY, cv::Mat& binary, int& retX, int& retY);

            void setImageHeader(QImage* qImg, const std::map<std::string, std::string> header);

            std::tr1::shared_ptr<Image2DDataUI16> convert(const cv::Mat& source);
            cv::Mat convert(std::tr1::shared_ptr<Image2DDataUI16> source);
        };

    }
}

#endif
