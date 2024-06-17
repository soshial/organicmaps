#pragma once

#include "base/assert.hpp"
#include "base/buffer_vector.hpp"
#include "base/shared_buffer_manager.hpp"

#include <tuple>  // std::tie

namespace dp
{
struct GlyphImage
{
  ~GlyphImage()
  {
    ASSERT_NOT_EQUAL(m_data.use_count(), 1, ("Probably you forgot to call Destroy()"));
  }

  // TODO(AB): Get rid of manual call to Destroy.
  void Destroy()
  {
    if (m_data != nullptr)
    {
      SharedBufferManager::instance().freeSharedBuffer(m_data->size(), m_data);
      m_data = nullptr;
    }
  }

  uint32_t m_width;
  uint32_t m_height;

  SharedBufferManager::shared_buffer_ptr_t m_data;
};

struct GlyphFontAndId
{
  int16_t fontIndex;
  uint16_t glyphId;

  bool operator<(GlyphFontAndId const & other) const
  {
    return std::tie(fontIndex, glyphId) < std::tie (other.fontIndex, other.glyphId);
  }
};


//using TGlyph = std::pair<int16_t /* fontIndex */, uint16_t /* glyphId */>;
// TODO(AB): Measure if 32 is the best value here.
using TGlyphs = buffer_vector<GlyphFontAndId, 32>;

struct Glyph
{
  Glyph(GlyphImage && image, GlyphFontAndId key)
  : m_image(image), m_key(key)
  {}

  GlyphImage m_image;
  GlyphFontAndId m_key;
};
}  // namespace dp

