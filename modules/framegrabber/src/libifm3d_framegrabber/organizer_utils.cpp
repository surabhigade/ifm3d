#include <ifm3d/fg/organizer_utils.h>
#include <glog/logging.h>
#include <ifm3d/device/err.h>

constexpr auto CHUNK_OFFSET_CHUNK_SIZE = 0x0004;
constexpr auto CHUNK_OFFSET_HEADER_SIZE = 0x0008;
constexpr auto CHUNK_OFFSET_HEADER_VERSION = 0x000C;
constexpr auto CHUNK_OFFSET_IMAGE_WIDTH = 0x0010;
constexpr auto CHUNK_OFFSET_IMAGE_HEIGHT = 0x0014;
constexpr auto CHUNK_OFFSET_PIXEL_FORMAT = 0x0018;
constexpr auto CHUNK_OFFSET_TIME_STAMP = 0x001C;
constexpr auto CHUNK_OFFSET_TIME_STAMP_SEC = 0x0028;
constexpr auto CHUNK_OFFSET_TIME_STAMP_NSEC = 0x002C;

std::size_t
ifm3d::get_format_size(ifm3d::pixel_format fmt)
{
  switch (fmt)
    {
    case pixel_format::FORMAT_8U:
    case pixel_format::FORMAT_8S:
      return 1;

    case pixel_format::FORMAT_16U:
    case pixel_format::FORMAT_16S:
    case pixel_format::FORMAT_16U2:
      return 2;

    case pixel_format::FORMAT_32F3:
    case pixel_format::FORMAT_32U:
    case pixel_format::FORMAT_32S:
    case pixel_format::FORMAT_32F:
      return 4;

    case pixel_format::FORMAT_64U:
    case pixel_format::FORMAT_64F:
      return 8;

    default:
      LOG(ERROR) << "Invalid pixel format => " << static_cast<uint32_t>(fmt);
      throw ifm3d::Error(IFM3D_PIXEL_FORMAT_ERROR);
    }
}

std::size_t
ifm3d::get_format_channels(ifm3d::pixel_format fmt)
{
  switch (fmt)
    {
    case pixel_format::FORMAT_8U:
    case pixel_format::FORMAT_8S:
    case pixel_format::FORMAT_16U:
    case pixel_format::FORMAT_16S:
    case pixel_format::FORMAT_32U:
    case pixel_format::FORMAT_32S:
    case pixel_format::FORMAT_32F:
    case pixel_format::FORMAT_64U:
    case pixel_format::FORMAT_64F:
      return 1;

    case pixel_format::FORMAT_16U2:
      return 2;

    case pixel_format::FORMAT_32F3:
      return 3;

    default:
      LOG(ERROR) << "Invalid pixel format => " << static_cast<uint32_t>(fmt);
      throw ifm3d::Error(IFM3D_PIXEL_FORMAT_ERROR);
    }
}

ifm3d::Image
ifm3d::create_image(const std::vector<std::uint8_t>& data,
                    std::size_t idx,
                    std::size_t width,
                    std::size_t height)
{
  auto fmt = get_chunk_format(data, idx);
  std::size_t pixeldata_offset = get_chunk_pixeldata_offset(data, idx);
  return ifm3d::create_image(data, idx + pixeldata_offset, width, height, fmt);
}

ifm3d::Image
ifm3d::create_image(const std::vector<std::uint8_t>& data,
                    std::size_t idx,
                    std::size_t width,
                    std::size_t height,
                    pixel_format fmt)
{
  uint32_t nchan = get_format_channels(fmt);
  std::size_t fsize = get_format_size(fmt);

  ifm3d::Image image(width, height, nchan, fmt);

  std::size_t incr = fsize * nchan;
  std::size_t npts = width * height;
  uint8_t* ptr = image.ptr<uint8_t>(0);

  const auto start = data.data() + idx;

#if !defined(_WIN32) && __BYTE_ORDER == __BIG_ENDIAN
  for (std::size_t i = 0; i < npts; ++i, idx += incr)
    {
      std::reverse_copy(start + i * fsize, start + (i + 1) * fsize, ptr + i);
    }
#else
  std::copy(start, start + npts * fsize, ptr);
#endif

  return image;
}

template <typename T>
static ifm3d::Image
create_xyz_image(const std::vector<std::uint8_t>& data,
                 std::size_t xidx,
                 std::size_t yidx,
                 std::size_t zidx,
                 std::size_t width,
                 std::size_t height,
                 ifm3d::pixel_format fmt,
                 const std::optional<ifm3d::Image>& mask)
{
  std::size_t incr = sizeof(T);
  std::size_t npts = width * height;

  ifm3d::Image im(width, height, 3, fmt);

  int col = 0;
  int row = -1;
  int xyz_col = 0;

  T* xyz_ptr;
  T x_, y_, z_;

  constexpr T bad_pixel = 0;

  for (std::size_t i = 0; i < npts;
       ++i, xidx += incr, yidx += incr, zidx += incr)
    {
      col = i % width;
      xyz_col = col * 3;
      if (col == 0)
        {
          row += 1;
          xyz_ptr = im.ptr<T>(row);
        }

      x_ = ifm3d::mkval<T>(data.data() + zidx);
      y_ = ifm3d::mkval<T>(data.data() + xidx);
      z_ = ifm3d::mkval<T>(data.data() + yidx);

      if (mask.has_value() && mask.value().at<uint8_t>(row, col) != 0)
        {
          xyz_ptr[xyz_col] = bad_pixel;
          xyz_ptr[xyz_col + 1] = bad_pixel;
          xyz_ptr[xyz_col + 2] = bad_pixel;
        }
      else
        {
          xyz_ptr[xyz_col] = x_;
          xyz_ptr[xyz_col + 1] = y_;
          xyz_ptr[xyz_col + 2] = z_;
        }
    }

  return im;
}

ifm3d::Image
ifm3d::create_xyz_image(const std::vector<std::uint8_t>& data,
                        std::size_t xidx,
                        std::size_t yidx,
                        std::size_t zidx,
                        std::size_t width,
                        std::size_t height,
                        ifm3d::pixel_format fmt,
                        const std::optional<Image>& mask)
{
  switch (fmt)
    {
    case ifm3d::pixel_format::FORMAT_8U:
      return ::create_xyz_image<std::uint8_t>(data,
                                              xidx,
                                              yidx,
                                              zidx,
                                              width,
                                              height,
                                              fmt,
                                              mask);
    case pixel_format::FORMAT_8S:
      return ::create_xyz_image<std::int8_t>(data,
                                             xidx,
                                             yidx,
                                             zidx,
                                             width,
                                             height,
                                             fmt,
                                             mask);
    case pixel_format::FORMAT_16U:
    case pixel_format::FORMAT_16U2:
      return ::create_xyz_image<std::uint16_t>(data,
                                               xidx,
                                               yidx,
                                               zidx,
                                               width,
                                               height,
                                               fmt,
                                               mask);
    case pixel_format::FORMAT_16S:
      return ::create_xyz_image<std::int16_t>(data,
                                              xidx,
                                              yidx,
                                              zidx,
                                              width,
                                              height,
                                              fmt,
                                              mask);
    case pixel_format::FORMAT_32U:
      return ::create_xyz_image<std::uint32_t>(data,
                                               xidx,
                                               yidx,
                                               zidx,
                                               width,
                                               height,
                                               fmt,
                                               mask);
    case pixel_format::FORMAT_32S:
      return ::create_xyz_image<std::int32_t>(data,
                                              xidx,
                                              yidx,
                                              zidx,
                                              width,
                                              height,
                                              fmt,
                                              mask);
    case pixel_format::FORMAT_64U:
      return ::create_xyz_image<uint64_t>(data,
                                          xidx,
                                          yidx,
                                          zidx,
                                          width,
                                          height,
                                          fmt,
                                          mask);
    case pixel_format::FORMAT_32F:
    case pixel_format::FORMAT_32F3:
      return ::create_xyz_image<float>(data,
                                       xidx,
                                       yidx,
                                       zidx,
                                       width,
                                       height,
                                       fmt,
                                       mask);
    case pixel_format::FORMAT_64F:
      return ::create_xyz_image<double>(data,
                                        xidx,
                                        yidx,
                                        zidx,
                                        width,
                                        height,
                                        fmt,
                                        mask);

    default:
      LOG(ERROR) << "Invalid pixel format => " << static_cast<uint32_t>(fmt);
      throw ifm3d::Error(IFM3D_PIXEL_FORMAT_ERROR);
    }
}

std::map<ifm3d::image_chunk, std::size_t>
ifm3d::get_image_chunks(const std::vector<std::uint8_t>& data,
                        std::size_t start_idx)
{
  std::map<image_chunk, std::size_t> chunks;

  std::size_t idx = start_idx; // start of first chunk
  std::size_t size = data.size() - 6;

  while (idx < size)
    {
      image_chunk chunk =
        static_cast<image_chunk>(mkval<std::uint32_t>(data.data() + idx));

      chunks[chunk] = idx;

      // move to the beginning of the next chunk
      std::uint32_t incr = mkval<std::uint32_t>(data.data() + idx + 4);
      if (incr <= 0)
        {
          LOG(WARNING) << "Next chunk is supposedly " << incr
                       << " bytes from the current one ... failing!";
          break;
        }
      idx += incr;
    }

  return chunks;
}

auto
ifm3d::find_metadata_chunk(std::map<ifm3d::image_chunk, std::size_t> chunks)
  -> decltype(chunks.end())
{
  // to get the metadata we use the confidence image for 3d and
  // the jpeg image for 2d

  auto metachunk = chunks.find(ifm3d::image_chunk::CONFIDENCE);

  if (metachunk == chunks.end())
    {
      metachunk = chunks.find(ifm3d::image_chunk::JPEG);
    }

  if (metachunk == chunks.end())
    {
      metachunk = chunks.find(ifm3d::image_chunk::ALGO_DEBUG);
    }

  return metachunk;
}

std::tuple<uint32_t, uint32_t>
ifm3d::get_image_size(const std::vector<std::uint8_t>& data, std::size_t idx)
{
  if (idx + CHUNK_OFFSET_IMAGE_HEIGHT + sizeof(std::uint32_t) >= data.size())
    {
      return std::make_tuple(0, 0);
    }

  std::uint32_t width = std::max(
    1u,
    ifm3d::mkval<std::uint32_t>(data.data() + idx + CHUNK_OFFSET_IMAGE_WIDTH));
  std::uint32_t height =
    std::max(1u,
             ifm3d::mkval<std::uint32_t>(data.data() + idx +
                                         CHUNK_OFFSET_IMAGE_HEIGHT));

  return std::make_tuple(width, height);
}

ifm3d::pixel_format
ifm3d::get_chunk_format(const std::vector<std::uint8_t>& data, std::size_t idx)
{
  return static_cast<pixel_format>(
    mkval<std::uint32_t>(data.data() + idx + CHUNK_OFFSET_PIXEL_FORMAT));
}

std::vector<ifm3d::TimePointT>
ifm3d::get_chunk_timestamps(const std::vector<uint8_t>& data, std::size_t idx)
{
  std::vector<TimePointT> timestamps;
  const std::uint32_t header_version = ifm3d::mkval<std::uint32_t>(
    data.data() + idx + CHUNK_OFFSET_HEADER_VERSION);
  if (header_version > 1)
    {
      // Retrieve the timespamp information from the chunk
      const std::uint32_t timestampSec = ifm3d::mkval<std::uint32_t>(
        data.data() + idx + CHUNK_OFFSET_TIME_STAMP_SEC);
      const std::uint32_t timestampNsec = ifm3d::mkval<std::uint32_t>(
        data.data() + idx + CHUNK_OFFSET_TIME_STAMP_NSEC);
      // convert the time stamp into a TimePointT
      timestamps.push_back(
        ifm3d::TimePointT{std::chrono::seconds{timestampSec} +
                          std::chrono::nanoseconds{timestampNsec}});

      // O3X device provides an offeset in Usec releative to timestamp
      // calculated as time_stamp_[0]
      const std::uint32_t ethernetTimeinUsecOffset =
        ifm3d::mkval<std::uint32_t>(data.data() + idx +
                                    CHUNK_OFFSET_TIME_STAMP);
      timestamps.push_back(ifm3d::TimePointT{
        timestamps[0] + std::chrono::microseconds{ethernetTimeinUsecOffset}});
    }
  else
    {
      // There is no *big* time stamp in chunk version 1
      timestamps.push_back(std::chrono::system_clock::now());
    }

  return timestamps;
}

std::size_t
ifm3d::get_chunk_pixeldata_offset(const std::vector<std::uint8_t>& data,
                                  std::size_t idx)
{
  if (idx + CHUNK_OFFSET_HEADER_SIZE + sizeof(std::uint32_t) >= data.size())
    {
      return 0;
    }

  return ifm3d::mkval<std::uint32_t>(data.data() + idx +
                                     CHUNK_OFFSET_HEADER_SIZE);
}

std::size_t
ifm3d::get_chunk_pixeldata_size(const std::vector<std::uint8_t>& data,
                                std::size_t idx)
{
  if (idx + CHUNK_OFFSET_CHUNK_SIZE + sizeof(std::uint32_t) >= data.size())
    {
      return 0;
    }

  auto header_size =
    ifm3d::mkval<std::uint32_t>(data.data() + idx + CHUNK_OFFSET_HEADER_SIZE);

  auto chunk_size =
    ifm3d::mkval<std::uint32_t>(data.data() + idx + CHUNK_OFFSET_CHUNK_SIZE);

  return chunk_size - header_size;
}

void
ifm3d::mask_image(ifm3d::Image& image, const ifm3d::Image& mask)
{
  switch (image.dataFormat())
    {
    case ifm3d::pixel_format::FORMAT_8U:
    case ifm3d::pixel_format::FORMAT_8S:
      image.setTo<std::uint8_t>(0, mask);
      break;

    case ifm3d::pixel_format::FORMAT_16U:
    case ifm3d::pixel_format::FORMAT_16S:
    case ifm3d::pixel_format::FORMAT_16U2:
      image.setTo<std::uint16_t>(0, mask);
      break;

    case ifm3d::pixel_format::FORMAT_32U:
    case ifm3d::pixel_format::FORMAT_32S:
    case ifm3d::pixel_format::FORMAT_32F:
    case ifm3d::pixel_format::FORMAT_32F3:
      image.setTo<std::uint32_t>(0, mask);
      break;

    case ifm3d::pixel_format::FORMAT_64U:
    case ifm3d::pixel_format::FORMAT_64F:
      image.setTo<std::uint64_t>(0, mask);
      break;

    default:
      break;
    }
}