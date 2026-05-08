#ifndef BAS_UI_ARCH_BITMAP_RESULT_HPP
#define BAS_UI_ARCH_BITMAP_RESULT_HPP

#include <wx/bitmap.h>

#include <optional>

enum ReasonCode {
    BMP_OK,
    BMP_NO_ART_ID,
    BMP_DISABLED,
    BMP_NONE,
    BMP_BAD_ART,
    BMP_BAD_ALT,
    BMP_BAD_ART_ALT,
    BMP_BAD_ASSET,
};

class BitmapResult {
  public:
    BitmapResult(ReasonCode reason = BMP_OK) : m_bitmap(std::nullopt), m_reason(reason) {}

    BitmapResult(const wxBitmap& bitmap, ReasonCode reason = BMP_OK)
        : m_bitmap(bitmap), m_reason(reason) {}

    BitmapResult(std::optional<wxBitmap> bitmap, ReasonCode reason = BMP_OK)
        : m_bitmap(bitmap), m_reason(reason) {}

    BitmapResult(const BitmapResult& other) //
        : m_bitmap(other.m_bitmap), m_reason(other.m_reason) {}
    BitmapResult& operator=(const BitmapResult& other) {
        m_bitmap = other.m_bitmap;
        m_reason = other.m_reason;
        return *this;
    }

    operator std::optional<wxBitmap>() const { return m_bitmap; }
    operator wxBitmap() const { return m_bitmap.value(); }

    // operator const wxBitmap*() const { return &m_bitmap.value(); }
    // operator wxBitmap*() { return &m_bitmap.value(); }

    // const wxBitmap*operator ->() const { return &m_bitmap.value(); }
    // wxBitmap*operator ->() { return &m_bitmap.value(); }

    // const wxBitmap& operator *() const { return m_bitmap.value(); }
    // wxBitmap& operator *() { return m_bitmap.value(); }

    std::optional<wxBitmap> getBitmap() const { return m_bitmap; }
    bool hasBitmap() const { return m_bitmap.has_value(); }
    void setBitmap(std::optional<wxBitmap> bitmap) { m_bitmap = bitmap; }
    void clearBitmap() { m_bitmap = std::nullopt; }

    wxBitmap* bitmapPtr() {
        if (m_reason == BMP_OK)
            if (m_bitmap && m_bitmap->IsOk())
                return &m_bitmap.value();
        return nullptr;
    }

    const wxBitmap* bitmapPtr() const {
        if (m_reason == BMP_OK)
            if (m_bitmap && m_bitmap->IsOk())
                return &m_bitmap.value();
        return nullptr;
    }

    ReasonCode getReason() const { return m_reason; }
    void setReason(ReasonCode reason) { m_reason = reason; }

    inline bool isOk() const { return m_bitmap && m_bitmap->IsOk() && m_reason == BMP_OK; }
    inline bool IsOk() const { return isOk(); }
    
    inline operator bool() const { return isOk(); }

  private:
    std::optional<wxBitmap> m_bitmap;
    ReasonCode m_reason{BMP_OK};
};

#endif
