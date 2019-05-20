#pragma once
#include <cstdint>

namespace ImageFileFormats
{
	enum color_format
	{
		RGB,
		RGBA,
		BGR,
		BGRA,
		BW
	};

	enum file_type
	{
		BMP,
		PNG,
		JPG,
		HDR,
		Not_Present,
		Unknown
	};

	// This is for the structures alignment
#pragma pack(push)
#pragma pack(1)

//=================================BMP=================================

	struct bmp_header_structure
	{
		// BMP header (14 bytes)
		uint16_t  type;             // Magic identifier: 0x4d42
		uint32_t  size;             // File size in bytes
		uint16_t  reserved1;        // Not used
		uint16_t  reserved2;        // Not used
		uint32_t  offset;           // Offset to image data in bytes from beginning of file
	};

	// DIB header
	struct dib_header_12
	{
		uint32_t  dib_header_size;  
		uint16_t  width_px;         
		uint16_t  height_px;        
		uint16_t  plane_count;      
		uint16_t  bits_per_pixel;   
	};

	struct dib_header_16
	{
		uint32_t  dib_header_size;  
		int32_t   width_px;         
		int32_t   height_px;        
		uint16_t  plane_count;      
		uint16_t  bits_per_pixel;   
		uint32_t  compression;      
	};

	struct dib_header_40
	{
		uint32_t  dib_header_size;  // DIB Header size in bytes (40 bytes)
		int32_t   width_px;         // Width of the image
		int32_t   height_px;        // Height of image
		uint16_t  plane_count;      // Number of color planes
		uint16_t  bits_per_pixel;   // Bits per pixel
		uint32_t  compression;      // Compression type
		uint32_t  image_size_bytes; // Image size in bytes
		int32_t   x_resolution_ppm; // Pixels per meter
		int32_t   y_resolution_ppm; // Pixels per meter
		uint32_t  color_count;      // Number of colors  
		uint32_t  important_colors; // Important colors 
	};

	struct dib_header_52
	{
		uint32_t        dib_header_size;
		int32_t         width_px;
		int32_t         height_px;
		uint16_t        plane_count;
		uint16_t        bits_per_pixel;
		uint32_t        compression;
		uint32_t        image_size_bytes;
		int32_t         x_resolution_ppm;
		int32_t         y_resolution_ppm;
		uint32_t        color_count;
		uint32_t        important_colors;
		uint32_t        red_mask;
		uint32_t        green_mask;
		uint32_t        blue_mask;
	};

	// Use 52 for 56 and 64 as well

	struct dib_header_108
	{
		uint32_t        dib_header_size;
		int32_t         width_px;
		int32_t         height_px;
		uint16_t        plane_count;
		uint16_t        bits_per_pixel;
		uint32_t        compression;
		uint32_t        image_size_bytes;
		int32_t         x_resolution_ppm;
		int32_t         y_resolution_ppm;
		uint32_t        color_count;
		uint32_t        important_colors;
		uint32_t        red_mask;
		uint32_t        green_mask;
		uint32_t        blue_mask;
		uint32_t        alpha_mask;
		uint32_t        color_space_type;
		uint32_t		unused[16];
		uint32_t        gamma_red;
		uint32_t        gamma_green;
		uint32_t        gamma_blue;
	};

	struct dib_header_124
	{
		uint32_t        dib_header_size;
		int32_t         width_px;
		int32_t         height_px;
		uint16_t        plane_count;
		uint16_t        bits_per_pixel;
		uint32_t        compression;
		uint32_t        image_size_bytes;
		int32_t         x_resolution_ppm;
		int32_t         y_resolution_ppm;
		uint32_t        color_count;
		uint32_t        important_colors;
		uint32_t        red_mask;
		uint32_t        green_mask;
		uint32_t        blue_mask;
		uint32_t        alpha_mask;
		uint32_t        color_space_type;
		uint32_t		unused[16];
		uint32_t        gamma_red;
		uint32_t        gamma_green;
		uint32_t        gamma_blue;
		uint32_t        intent;
		uint32_t        profile_data;
		uint32_t        profile_profile;
		uint32_t        reserved;
	};

	//=====================================================================

#pragma pack(pop)
}
