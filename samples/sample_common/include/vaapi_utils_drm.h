/******************************************************************************\
Copyright (c) 2005-2019, Intel Corporation
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

This sample was distributed or derived from the Intel's Media Samples package.
The original version of this sample may be obtained from https://software.intel.com/en-us/intel-media-server-studio
or https://software.intel.com/en-us/media-client-solutions-support.
\**********************************************************************************/

#ifndef __VAAPI_UTILS_DRM_H__
#define __VAAPI_UTILS_DRM_H__

#if defined(LIBVA_DRM_SUPPORT)

#include <va/va_drm.h>
#include <va/va_drmcommon.h>
#include "vaapi_utils.h"
#include "vaapi_allocator.h"

class drmRenderer;

class DRMLibVA : public CLibVA
{
public:
    DRMLibVA(const std::string& devicePath = "", int type = MFX_LIBVA_DRM);
    virtual ~DRMLibVA(void);

    inline int getFD() { return m_fd; }

protected:
    int m_fd;
    MfxLoader::VA_DRMProxy m_vadrmlib;

private:
    DISALLOW_COPY_AND_ASSIGN(DRMLibVA);
};

/**
* struct hdr_metadata_infoframe - HDR Metadata Infoframe Data.
*
* HDR Metadata Infoframe as per CTA 861.G spec. This is expected
* to match exactly with the spec.
*
* Userspace is expected to pass the metadata information as per
* the format described in this structure.
*/
struct hdr_metadata_infoframe {
    /**
    * @eotf: Electro-Optical Transfer Function (EOTF)
    * used in the stream.
    */
    uint8_t eotf;
    /**
    * @metadata_type: Static_Metadata_Descriptor_ID.
    */
    uint8_t metadata_type;
    /**
    * @display_primaries: Color Primaries of the Data.
    * These are coded as unsigned 16-bit values in units of
    * 0.00002, where 0x0000 represents zero and 0xC350
    * represents 1.0000.
    * @display_primaries.x: X cordinate of color primary.
    * @display_primaries.y: Y cordinate of color primary.
    */
    struct {
        uint16_t x, y;
        } display_primaries[3];
    /**
    * @white_point: White Point of Colorspace Data.
    * These are coded as unsigned 16-bit values in units of
    * 0.00002, where 0x0000 represents zero and 0xC350
    * represents 1.0000.
    * @white_point.x: X cordinate of whitepoint of color primary.
    * @white_point.y: Y cordinate of whitepoint of color primary.
    */
    struct {
        uint16_t x, y;
        } white_point;
    /**
    * @max_display_mastering_luminance: Max Mastering Display Luminance.
    * This value is coded as an unsigned 16-bit value in units of 1 cd/m2,
    * where 0x0001 represents 1 cd/m2 and 0xFFFF represents 65535 cd/m2.
    */
    uint16_t max_display_mastering_luminance;
    /**
    * @min_display_mastering_luminance: Min Mastering Display Luminance.
    * This value is coded as an unsigned 16-bit value in units of
    * 0.0001 cd/m2, where 0x0001 represents 0.0001 cd/m2 and 0xFFFF
    * represents 6.5535 cd/m2.
    */
    uint16_t min_display_mastering_luminance;
    /**
    * @max_cll: Max Content Light Level.
    * This value is coded as an unsigned 16-bit value in units of 1 cd/m2,
    * where 0x0001 represents 1 cd/m2 and 0xFFFF represents 65535 cd/m2.
    */
    uint16_t max_cll;
    /**
    * @max_fall: Max Frame Average Light Level.
    * This value is coded as an unsigned 16-bit value in units of 1 cd/m2,
    * where 0x0001 represents 1 cd/m2 and 0xFFFF represents 65535 cd/m2.
    */
    uint16_t max_fall;
};

/**
* struct hdr_output_metadata - HDR output metadata
*
* Metadata Information to be passed from userspace
*/
struct hdr_output_metadata {
    /**
    * @metadata_type: Static_Metadata_Descriptor_ID.
    */
    uint32_t metadata_type;

    /**
    * @hdmi_metadata_type1: HDR Metadata Infoframe.
    */
    union {
        struct hdr_metadata_infoframe hdmi_metadata_type1;
    };
};

struct drm_hdr_metadata {
    struct hdr_output_metadata data;
    uint32_t hdr_blob_id;
    uint32_t mode_blob_id;
};


class drmRenderer : public vaapiAllocatorParams::Exporter
{
public:
    drmRenderer(int fd, mfxI32 monitorType);
    virtual ~drmRenderer();

    virtual mfxStatus render(mfxFrameSurface1 * pSurface);

    // vaapiAllocatorParams::Exporter methods
    virtual void* acquire(mfxMemId mid);
    virtual void release(mfxMemId mid, void * mem);

    static uint32_t getConnectorType(mfxI32 monitor_type);
    static const msdk_char* getConnectorName(uint32_t connector_type);

private:
    drmModeObjectPropertiesPtr getProperties(int fd, int objectId, int32_t objectTypeId);
    bool getConnector(drmModeRes *resource, uint32_t connector_type);
    bool setupConnection(drmModeRes *resource, drmModeConnector* connector);
    bool getPlane();

    bool getConnectorProperties(int fd, int connectorId);
    bool getCRTCProperties(int fd, int crtcId);

    bool setMaster();
    void dropMaster();
    bool restore();

    struct drm_hdr_metadata m_hdr_metadata;
    uint32_t getConnectorPropertyId(const char *propNameToFind);
    uint32_t getCRTCPropertyId(const char *propNameToFind);
    void drm_send_bt2020_colorspace();
    void drm_send_hdr(mfxFrameSurface1 * pSurface, bool enableHDR);
    void drm_destroy_hdrinfo();


    const MfxLoader::DRM_Proxy m_drmlib;
    const MfxLoader::DrmIntel_Proxy m_drmintellib;

    int m_fd;
    bool m_sentHDR;
    uint32_t m_connector_type;
    uint32_t m_connectorID;
    uint32_t m_encoderID;
    uint32_t m_crtcID;
    uint32_t m_crtcIndex;
    uint32_t m_planeID;
    uint32_t m_planeFMT;
    uint32_t m_needed_pixelFMT;
    drmModeModeInfo m_mode;
    drmModeCrtcPtr m_crtc;
    drmModeObjectPropertiesPtr m_PlaneProperties;
    drmModeObjectPropertiesPtr m_ConnectorProperties;
    drmModeObjectPropertiesPtr m_crtcProperties;
    drm_intel_bufmgr* m_bufmgr;
    bool m_overlay_wrn;
    mfxFrameSurface1 * m_pCurrentRenderTargetSurface;

private:
    DISALLOW_COPY_AND_ASSIGN(drmRenderer);
};

#endif // #if defined(LIBVA_DRM_SUPPORT)

#endif // #ifndef __VAAPI_UTILS_DRM_H__
