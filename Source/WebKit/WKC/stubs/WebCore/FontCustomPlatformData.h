#pragma once

#include <WebCore/FontPlatformData.h>
#include <WebCore/RenderingResourceIdentifier.h>
#include <wtf/Forward.h>
#include <wtf/Noncopyable.h>
#include <wtf/RefCounted.h>
#include <wtf/StdLibExtras.h>
#include <wtf/TZoneMallocInlines.h>

namespace WebCore {

class SharedBuffer;
class FontDescription;
class FontCreationContext;
enum class FontTechnology : uint8_t;

template<typename> class FontTaggedSettings;
using FontFeatureSettings = FontTaggedSettings<int>;

struct FontCustomPlatformSerializedData {
    Ref<SharedBuffer> fontFaceData;
    String itemInCollection;
    RenderingResourceIdentifier renderingResourceIdentifier;
};

struct FontCustomPlatformData : public RefCounted<FontCustomPlatformData> {
    WTF_MAKE_TZONE_ALLOCATED_INLINE(FontCustomPlatformData);
    WTF_MAKE_NONCOPYABLE(FontCustomPlatformData);
public:
    WEBCORE_EXPORT static RefPtr<FontCustomPlatformData> create(SharedBuffer&, const String&);
    WEBCORE_EXPORT static RefPtr<FontCustomPlatformData> createMemorySafe(SharedBuffer&, const String&);

    FontCustomPlatformData(FontPlatformData::CreationData&& data, RenderingResourceIdentifier identifier)
        : creationData(std::move(data))
        , m_renderingResourceIdentifier(identifier)
    {
    }

    WEBCORE_EXPORT ~FontCustomPlatformData();

    FontPlatformData fontPlatformData(const FontDescription&, bool bold, bool italic, const FontCreationContext&);

    WEBCORE_EXPORT FontCustomPlatformSerializedData serializedData() const;
    WEBCORE_EXPORT static std::optional<Ref<FontCustomPlatformData>> tryMakeFromSerializationData(FontCustomPlatformSerializedData&&, bool);

    static bool supportsFormat(const String&);
    static bool supportsTechnology(const FontTechnology&);

    FontPlatformData::CreationData creationData;
    RenderingResourceIdentifier m_renderingResourceIdentifier;
};

} // namespace WebCore
