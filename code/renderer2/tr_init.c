/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
// tr_init.c -- functions that are not called every frame

#include "tr_local.h"

#include "tr_dsa.h"

glconfig_t  glConfig;
glRefConfig_t glRefConfig;
qboolean    textureFilterAnisotropic = qfalse;
int         maxAnisotropy = 0;

glstate_t	glState;

static void GfxInfo_f( void );
static void GfxMemInfo_f( void );

cvar_t	*r_flareSize;
cvar_t	*r_flareFade;
cvar_t	*r_flareCoeff;

cvar_t	*r_railWidth;
cvar_t	*r_railCoreWidth;
cvar_t	*r_railSegmentLength;

cvar_t	*r_ignore;

cvar_t	*r_detailTextures;

cvar_t	*r_znear;
cvar_t	*r_zfar;
cvar_t	*r_zproj;
cvar_t	*r_stereoSeparation;

cvar_t	*r_skipBackEnd;

cvar_t	*r_anaglyphMode;

cvar_t	*r_greyscale;
cvar_t  *r_paletteMode;
cvar_t  *r_seeThroughWalls;

cvar_t	*r_ignorehwgamma;
cvar_t	*r_measureOverdraw;

cvar_t	*r_fastsky;
cvar_t	*r_drawSun;
cvar_t	*r_dynamiclight;
cvar_t	*r_dlightBacks;

cvar_t	*r_lodbias;
cvar_t	*r_lodscale;
#ifdef USE_LAZY_LOAD
cvar_t	*r_lazyLoad;
#endif

cvar_t	*r_norefresh;
cvar_t	*r_drawentities;
cvar_t	*r_drawworld;
cvar_t	*r_speeds;
cvar_t	*r_fullbright;
cvar_t	*r_novis;
cvar_t	*r_nocull;
cvar_t	*r_facePlaneCull;
cvar_t	*r_showcluster;
cvar_t	*r_nocurves;

cvar_t	*r_allowExtensions;

cvar_t	*r_ext_compressed_textures;
cvar_t	*r_ext_multitexture;
cvar_t	*r_ext_compiled_vertex_array;
cvar_t	*r_ext_texture_env_add;
cvar_t	*r_ext_texture_filter_anisotropic;
cvar_t	*r_ext_max_anisotropy;

cvar_t  *r_ext_framebuffer_object;
cvar_t  *r_ext_texture_float;
cvar_t  *r_ext_framebuffer_multisample;
cvar_t  *r_arb_seamless_cube_map;
cvar_t  *r_arb_vertex_array_object;
cvar_t  *r_ext_direct_state_access;

cvar_t  *r_cameraExposure;

cvar_t  *r_externalGLSL;

cvar_t  *r_hdr;
cvar_t  *r_floatLightmap;
cvar_t  *r_postProcess;

cvar_t  *r_toneMap;
cvar_t  *r_forceToneMap;
cvar_t  *r_forceToneMapMin;
cvar_t  *r_forceToneMapAvg;
cvar_t  *r_forceToneMapMax;

cvar_t  *r_autoExposure;
cvar_t  *r_forceAutoExposure;
cvar_t  *r_forceAutoExposureMin;
cvar_t  *r_forceAutoExposureMax;

cvar_t  *r_depthPrepass;
cvar_t  *r_ssao;

cvar_t  *r_normalMapping;
cvar_t  *r_specularMapping;
cvar_t  *r_deluxeMapping;
cvar_t  *r_parallaxMapping;
cvar_t  *r_parallaxMapShadows;
cvar_t  *r_cubeMapping;
cvar_t  *r_cubemapSize;
cvar_t  *r_deluxeSpecular;
cvar_t  *r_pbr;
cvar_t  *r_baseNormalX;
cvar_t  *r_baseNormalY;
cvar_t  *r_baseParallax;
cvar_t  *r_baseSpecular;
cvar_t  *r_baseGloss;
cvar_t  *r_glossType;
cvar_t  *r_mergeLightmaps;
cvar_t  *r_dlightMode;
cvar_t  *r_pshadowDist;
cvar_t  *r_imageUpsample;
cvar_t  *r_imageUpsampleMaxSize;
cvar_t  *r_imageUpsampleType;
cvar_t  *r_genNormalMaps;
cvar_t  *r_forceSun;
cvar_t  *r_forceSunLightScale;
cvar_t  *r_forceSunAmbientScale;
cvar_t  *r_sunlightMode;
cvar_t  *r_drawSunRays;
cvar_t  *r_sunShadows;
cvar_t  *r_shadowFilter;
cvar_t  *r_shadowBlur;
cvar_t  *r_shadowMapSize;
cvar_t  *r_shadowCascadeZNear;
cvar_t  *r_shadowCascadeZFar;
cvar_t  *r_shadowCascadeZBias;
cvar_t  *r_ignoreDstAlpha;

cvar_t	*r_ignoreGLErrors;
cvar_t	*r_logFile;

cvar_t	*r_stencilbits;
cvar_t	*r_texturebits;
cvar_t  *r_ext_multisample;

cvar_t	*r_drawBuffer;
cvar_t	*r_lightmap;
cvar_t	*r_vertexLight;
cvar_t	*r_shadows;
cvar_t	*r_flares;
cvar_t	*r_nobind;
cvar_t	*r_singleShader;
cvar_t	*r_roundImagesDown;
cvar_t	*r_colorMipLevels;
cvar_t	*r_picmip;
cvar_t	*r_showtris;
cvar_t	*r_showsky;
cvar_t	*r_shownormals;
cvar_t	*r_finish;
cvar_t	*r_clear;
cvar_t	*r_textureMode;
cvar_t	*r_offsetFactor;
cvar_t	*r_offsetUnits;
cvar_t	*r_gamma;
cvar_t	*r_intensity;
cvar_t	*r_lockpvs;
cvar_t	*r_noportals;
cvar_t	*r_portalOnly;

cvar_t	*r_subdivisions;
cvar_t	*r_lodCurveError;

cvar_t	*r_overBrightBits;
cvar_t	*r_mapOverBrightBits;

cvar_t	*r_debugSurface;
cvar_t	*r_simpleMipMaps;

cvar_t	*r_showImages;

cvar_t	*r_ambientScale;
cvar_t	*r_directedScale;
cvar_t	*r_debugLight;
cvar_t	*r_debugSort;
cvar_t	*r_printShaders;
cvar_t	*r_saveFontData;

cvar_t	*r_marksOnTriangleMeshes;

cvar_t	*r_aviMotionJpegQuality;
cvar_t	*r_screenshotJpegQuality;

cvar_t	*r_maxpolys;
int		max_polys;
cvar_t	*r_maxpolyverts;
int		max_polyverts;

float dvrXScale = 1;
float dvrYScale = 1;
float dvrXOffset = 0;
float dvrYOffset = 0;

cvar_t  *r_inputShader;
cvar_t  *r_cursorShader;

/*
** InitOpenGL
**
** This function is responsible for initializing a valid OpenGL subsystem.  This
** is done by calling GLimp_Init (which gives us a working OGL subsystem) then
** setting variables, checking GL constants, and reporting the gfx system config
** to the user.
*/
static void InitOpenGL( void )
{
	char renderer_buffer[1024];

	//
	// initialize OS specific portions of the renderer
	//
	// GLimp_Init directly or indirectly references the following cvars:
	//		- r_fullscreen
	//		- r_mode
	//		- r_(color|depth|stencil)bits
	//		- r_ignorehwgamma
	//		- r_gamma
	//
	
	if ( glConfig.vidWidth == 0 )
	{
		GLint		temp;
		
		ri.GLimp_Init( &glConfig );
		GLimp_InitExtraExtensions();

		glConfig.textureEnvAddAvailable = qtrue;

		strcpy( renderer_buffer, glConfig.renderer_string );
		Q_strlwr( renderer_buffer );

		// OpenGL driver constants
		qglGetIntegerv( GL_MAX_TEXTURE_SIZE, &temp );
		glConfig.maxTextureSize = temp;

		// stubbed or broken drivers may have reported 0...
		if ( glConfig.maxTextureSize <= 0 ) 
		{
			glConfig.maxTextureSize = 0;
		}

		ri.CL_SetScaling( 1.0, glConfig.vidWidth, glConfig.vidHeight );

		qglGetIntegerv( GL_MAX_TEXTURE_IMAGE_UNITS, &temp );
		glConfig.numTextureUnits = temp;

		// reserve 160 components for other uniforms
		qglGetIntegerv( GL_MAX_VERTEX_UNIFORM_COMPONENTS, &temp );
		glRefConfig.glslMaxAnimatedBones = Com_Clamp( 0, IQM_MAX_JOINTS, ( temp - 160 ) / 16 );
		if ( glRefConfig.glslMaxAnimatedBones < 12 ) {
			glRefConfig.glslMaxAnimatedBones = 0;
		}
	}

	// check for GLSL function textureCubeLod()
	if ( r_cubeMapping->integer && !QGL_VERSION_ATLEAST( 3, 0 ) ) {
		ri.Printf( PRINT_WARNING, "WARNING: Disabled r_cubeMapping because it requires OpenGL 3.0\n" );
		ri.Cvar_Set( "r_cubeMapping", "0" );
	}

	// set default state
	GL_SetDefaultState();
}


/*
==================
GL_CheckErrors
==================
*/
void GL_CheckErrs( char *file, int line ) {
	int		err;
	char	s[64];

	err = qglGetError();
	if ( err == GL_NO_ERROR ) {
		return;
	}
	if ( r_ignoreGLErrors->integer ) {
		return;
	}
	switch( err ) {
		case GL_INVALID_ENUM:
			strcpy( s, "GL_INVALID_ENUM" );
			break;
		case GL_INVALID_VALUE:
			strcpy( s, "GL_INVALID_VALUE" );
			break;
		case GL_INVALID_OPERATION:
			strcpy( s, "GL_INVALID_OPERATION" );
			break;
		case GL_STACK_OVERFLOW:
			strcpy( s, "GL_STACK_OVERFLOW" );
			break;
		case GL_STACK_UNDERFLOW:
			strcpy( s, "GL_STACK_UNDERFLOW" );
			break;
		case GL_OUT_OF_MEMORY:
			strcpy( s, "GL_OUT_OF_MEMORY" );
			break;
		default:
			Com_sprintf( s, sizeof(s), "%i", err);
			break;
	}

	ri.Error( ERR_FATAL, "GL_CheckErrors: %s in %s at line %d", s , file, line);
}


/* 
============================================================================== 
 
						SCREEN SHOTS 

NOTE TTimo
some thoughts about the screenshots system:
screenshots get written in fs_homepath + fs_gamedir
vanilla q3 .. baseq3/screenshots/ *.tga
team arena .. missionpack/screenshots/ *.tga

two commands: "screenshot" and "screenshotJPEG"
we use statics to store a count and start writing the first screenshot/screenshot????.tga (.jpg) available
(with FS_FileExists / FS_FOpenFileWrite calls)
FIXME: the statics don't get a reinit between fs_game changes

============================================================================== 
*/ 

/* 
================== 
RB_ReadPixels

Reads an image but takes care of alignment issues for reading RGB images.

Reads a minimum offset for where the RGB data starts in the image from
integer stored at pointer offset. When the function has returned the actual
offset was written back to address offset. This address will always have an
alignment of packAlign to ensure efficient copying.

Stores the length of padding after a line of pixels to address padlen

Return value must be freed with ri.Hunk_FreeTempMemory()
================== 
*/  

byte *RB_ReadPixels(int x, int y, int width, int height, size_t *offset, int *padlen)
{
	byte *buffer, *bufstart;
	int padwidth, linelen;
	GLint packAlign;
	
	qglGetIntegerv(GL_PACK_ALIGNMENT, &packAlign);
	
	linelen = width * 3;
	padwidth = PAD(linelen, packAlign);
	
	// Allocate a few more bytes so that we can choose an alignment we like
	buffer = ri.Hunk_AllocateTempMemory(padwidth * height + *offset + packAlign - 1);
	
	bufstart = PADP((intptr_t) buffer + *offset, packAlign);

	qglReadPixels(x, y, width, height, GL_RGB, GL_UNSIGNED_BYTE, bufstart);
	
	*offset = bufstart - buffer;
	*padlen = padwidth - linelen;
	
	return buffer;
}

/* 
================== 
RB_TakeScreenshot
================== 
*/  
void RB_TakeScreenshot(int x, int y, int width, int height, char *fileName)
{
	byte *allbuf, *buffer;
	byte *srcptr, *destptr;
	byte *endline, *endmem;
	byte temp;
	
	int linelen, padlen;
	size_t offset = 18, memcount;
		
	allbuf = RB_ReadPixels(x, y, width, height, &offset, &padlen);
	buffer = allbuf + offset - 18;
	
	Com_Memset (buffer, 0, 18);
	buffer[2] = 2;		// uncompressed type
	buffer[12] = width & 255;
	buffer[13] = width >> 8;
	buffer[14] = height & 255;
	buffer[15] = height >> 8;
	buffer[16] = 24;	// pixel size

	// swap rgb to bgr and remove padding from line endings
	linelen = width * 3;
	
	srcptr = destptr = allbuf + offset;
	endmem = srcptr + (linelen + padlen) * height;
	
	while(srcptr < endmem)
	{
		endline = srcptr + linelen;

		while(srcptr < endline)
		{
			temp = srcptr[0];
			*destptr++ = srcptr[2];
			*destptr++ = srcptr[1];
			*destptr++ = temp;
			
			srcptr += 3;
		}
		
		// Skip the pad
		srcptr += padlen;
	}

	memcount = linelen * height;

	// gamma correct
	if(glConfig.deviceSupportsGamma)
		R_GammaCorrect(allbuf + offset, memcount);

	ri.FS_WriteFile(fileName, buffer, memcount + 18);

	ri.Hunk_FreeTempMemory(allbuf);
#ifdef EMSCRIPTEN
	ri.Sys_DownloadLocalFile(fileName);
#endif
}

/* 
================== 
RB_TakeScreenshotJPEG
================== 
*/

void RB_TakeScreenshotJPEG(int x, int y, int width, int height, char *fileName)
{
	byte *buffer;
	size_t offset = 0, memcount;
	int padlen;

	buffer = RB_ReadPixels(x, y, width, height, &offset, &padlen);
	memcount = (width * 3 + padlen) * height;

	// gamma correct
	if(glConfig.deviceSupportsGamma)
		R_GammaCorrect(buffer + offset, memcount);

	ri.CL_SaveJPG(fileName, r_screenshotJpegQuality->integer, width, height, buffer + offset, padlen);
	ri.Hunk_FreeTempMemory(buffer);
#ifdef EMSCRIPTEN
	ri.Sys_DownloadLocalFile(fileName);
#endif
}

/*
==================
RB_TakeScreenshotCmd
==================
*/
const void *RB_TakeScreenshotCmd( const void *data ) {
	const screenshotCommand_t	*cmd;
	
	cmd = (const screenshotCommand_t *)data;

	// finish any 2D drawing if needed
	if(tess.numIndexes)
		RB_EndSurface();

	if (cmd->jpeg)
		RB_TakeScreenshotJPEG( cmd->x, cmd->y, cmd->width, cmd->height, cmd->fileName);
	else
		RB_TakeScreenshot( cmd->x, cmd->y, cmd->width, cmd->height, cmd->fileName);
	
	return (const void *)(cmd + 1);	
}

/*
==================
R_TakeScreenshot
==================
*/
#ifdef EMSCRIPTEN
void R_TakeScreenshot( int x, int y, int width, int height, char *name, qboolean jpeg, qboolean downloadAfter ) {
#else
void R_TakeScreenshot( int x, int y, int width, int height, char *name, qboolean jpeg ) {
#endif
;
	static char	fileName[MAX_OSPATH]; // bad things if two screenshots per frame?
	screenshotCommand_t	*cmd;

	cmd = R_GetCommandBuffer( sizeof( *cmd ) );
	if ( !cmd ) {
		return;
	}
	cmd->commandId = RC_SCREENSHOT;

	cmd->x = x;
	cmd->y = y;
	cmd->width = width;
	cmd->height = height;
	Q_strncpyz( fileName, name, sizeof(fileName) );
	cmd->fileName = fileName;
	cmd->jpeg = jpeg;
}

/* 
================== 
R_ScreenshotFilename
================== 
*/  
void R_ScreenshotFilename( int lastNumber, char *fileName ) {
	int		a,b,c,d;

	if ( lastNumber < 0 || lastNumber > 9999 ) {
		Com_sprintf( fileName, MAX_OSPATH, "screenshots/shot9999.tga" );
		return;
	}

	a = lastNumber / 1000;
	lastNumber -= a*1000;
	b = lastNumber / 100;
	lastNumber -= b*100;
	c = lastNumber / 10;
	lastNumber -= c*10;
	d = lastNumber;

	Com_sprintf( fileName, MAX_OSPATH, "screenshots/shot%i%i%i%i.tga"
		, a, b, c, d );
}

/* 
================== 
R_ScreenshotFilename
================== 
*/  
void R_ScreenshotFilenameJPEG( int lastNumber, char *fileName ) {
	int		a,b,c,d;

	if ( lastNumber < 0 || lastNumber > 9999 ) {
		Com_sprintf( fileName, MAX_OSPATH, "screenshots/shot9999.jpg" );
		return;
	}

	a = lastNumber / 1000;
	lastNumber -= a*1000;
	b = lastNumber / 100;
	lastNumber -= b*100;
	c = lastNumber / 10;
	lastNumber -= c*10;
	d = lastNumber;

	Com_sprintf( fileName, MAX_OSPATH, "screenshots/shot%i%i%i%i.jpg"
		, a, b, c, d );
}

/*
====================
R_LevelShot

levelshots are specialized 128*128 thumbnails for
the menu system, sampled down from full screen distorted images
====================
*/
void R_LevelShot( void ) {
	char		checkname[MAX_OSPATH];
	byte		*buffer;
	byte		*source, *allsource;
	byte		*src, *dst;
	size_t			offset = 0;
	int			padlen;
	int			x, y;
	int			r, g, b;
	float		xScale, yScale;
	int			xx, yy;

	Com_sprintf(checkname, sizeof(checkname), "levelshots/%s.tga", tr.world->baseName);

	allsource = RB_ReadPixels(0, 0, glConfig.vidWidth, glConfig.vidHeight, &offset, &padlen);
	source = allsource + offset;

	buffer = ri.Hunk_AllocateTempMemory(128 * 128*3 + 18);
	Com_Memset (buffer, 0, 18);
	buffer[2] = 2;		// uncompressed type
	buffer[12] = 128;
	buffer[14] = 128;
	buffer[16] = 24;	// pixel size

	// resample from source
	xScale = glConfig.vidWidth / 512.0f;
	yScale = glConfig.vidHeight / 384.0f;
	for ( y = 0 ; y < 128 ; y++ ) {
		for ( x = 0 ; x < 128 ; x++ ) {
			r = g = b = 0;
			for ( yy = 0 ; yy < 3 ; yy++ ) {
				for ( xx = 0 ; xx < 4 ; xx++ ) {
					src = source + (3 * glConfig.vidWidth + padlen) * (int)((y*3 + yy) * yScale) +
						3 * (int) ((x*4 + xx) * xScale);
					r += src[0];
					g += src[1];
					b += src[2];
				}
			}
			dst = buffer + 18 + 3 * ( y * 128 + x );
			dst[0] = b / 12;
			dst[1] = g / 12;
			dst[2] = r / 12;
		}
	}

	// gamma correct
	if ( glConfig.deviceSupportsGamma ) {
		R_GammaCorrect( buffer + 18, 128 * 128 * 3 );
	}

	ri.FS_WriteFile( checkname, buffer, 128 * 128*3 + 18 );

	ri.Hunk_FreeTempMemory(buffer);
	ri.Hunk_FreeTempMemory(allsource);

	ri.Printf( PRINT_ALL, "Wrote %s\n", checkname );
}

/* 
================== 
R_ScreenShot_f

screenshot
screenshot [silent]
screenshot [levelshot]
screenshot [filename]

Doesn't print the pacifier message if there is a second arg
================== 
*/  
void R_ScreenShot_f (void) {
	char	checkname[MAX_OSPATH];
	static	int	lastNumber = -1;
	qboolean	silent;

	if ( !strcmp( ri.Cmd_Argv(1), "levelshot" ) ) {
		R_LevelShot();
		return;
	}

	if ( !strcmp( ri.Cmd_Argv(1), "silent" ) ) {
		silent = qtrue;
	} else {
		silent = qfalse;
	}

	if ( ri.Cmd_Argc() == 2 && !silent ) {
		// explicit filename
		Com_sprintf( checkname, MAX_OSPATH, "screenshots/%s.tga", ri.Cmd_Argv( 1 ) );
	} else {
		// scan for a free filename

		// if we have saved a previous screenshot, don't scan
		// again, because recording demo avis can involve
		// thousands of shots
		if ( lastNumber == -1 ) {
			lastNumber = 0;
		}
		// scan for a free number
		for ( ; lastNumber <= 9999 ; lastNumber++ ) {
			R_ScreenshotFilename( lastNumber, checkname );

      if (!ri.FS_FileExists( checkname ))
      {
        break; // file doesn't exist
      }
		}

		if ( lastNumber >= 9999 ) {
			ri.Printf (PRINT_ALL, "ScreenShot: Couldn't create a file\n"); 
			return;
 		}

		lastNumber++;
	}

#ifdef EMSCRIPTEN
	R_TakeScreenshot( 0, 0, glConfig.vidWidth, glConfig.vidHeight, checkname, qfalse, qtrue );
#else
	R_TakeScreenshot( 0, 0, glConfig.vidWidth, glConfig.vidHeight, checkname, qfalse );
#endif

	if ( !silent ) {
		ri.Printf (PRINT_ALL, "Wrote %s\n", checkname);
	}
} 

void R_ScreenShotJPEG_f (void) {
	char		checkname[MAX_OSPATH];
	static	int	lastNumber = -1;
	qboolean	silent;

	if ( !strcmp( ri.Cmd_Argv(1), "levelshot" ) ) {
		R_LevelShot();
		return;
	}

	if ( !strcmp( ri.Cmd_Argv(1), "silent" ) ) {
		silent = qtrue;
	} else {
		silent = qfalse;
	}

	if ( ri.Cmd_Argc() == 2 && !silent ) {
		// explicit filename
		Com_sprintf( checkname, MAX_OSPATH, "screenshots/%s.jpg", ri.Cmd_Argv( 1 ) );
	} else {
		// scan for a free filename

		// if we have saved a previous screenshot, don't scan
		// again, because recording demo avis can involve
		// thousands of shots
		if ( lastNumber == -1 ) {
			lastNumber = 0;
		}
		// scan for a free number
		for ( ; lastNumber <= 9999 ; lastNumber++ ) {
			R_ScreenshotFilenameJPEG( lastNumber, checkname );

      if (!ri.FS_FileExists( checkname ))
      {
        break; // file doesn't exist
      }
		}

		if ( lastNumber == 10000 ) {
			ri.Printf (PRINT_ALL, "ScreenShot: Couldn't create a file\n"); 
			return;
 		}

		lastNumber++;
	}

#ifdef EMSCRIPTEN
	R_TakeScreenshot( 0, 0, glConfig.vidWidth, glConfig.vidHeight, checkname, qtrue, qtrue );
#else
	R_TakeScreenshot( 0, 0, glConfig.vidWidth, glConfig.vidHeight, checkname, qtrue );
#endif

	if ( !silent ) {
		ri.Printf (PRINT_ALL, "Wrote %s\n", checkname);
	}
} 

//============================================================================

/*
==================
R_ExportCubemaps
==================
*/
void R_ExportCubemaps(void)
{
	exportCubemapsCommand_t	*cmd;

	cmd = R_GetCommandBuffer(sizeof(*cmd));
	if (!cmd) {
		return;
	}
	cmd->commandId = RC_EXPORT_CUBEMAPS;
}


/*
==================
R_ExportCubemaps_f
==================
*/
void R_ExportCubemaps_f(void)
{
	R_ExportCubemaps();
}

//============================================================================
static GLuint videoPBO[2] = {0, 0};
//static GLuint fbo = 0;
static int fboIndex = 0;
GLuint depthrenderbuffer;
void RB_FastCapture(byte *data)
{
	byte				*cBuf;
	size_t				memcount, linelen;
	int				padwidth, padlen;
	GLint packAlign;
	fboIndex = (fboIndex + 1) % 2;

	// finish any 2D drawing if needed
	if(tess.numIndexes)
		RB_EndSurface();
	
	if(!videoPBO[0]) {
		qglGenBuffers(1, &videoPBO[0]);
		qglGenBuffers(1, &videoPBO[1]);
	}
/*
		qglGenFramebuffers(1, &fbo);
		qglGenRenderbuffers(1, &depthrenderbuffer);
	}
	
	qglBindFramebuffer(GL_FRAMEBUFFER, fbo);
	
	qglGenTextures(1, &tr.renderImage->texnum);

	// "Bind" the newly created texture : all future texture functions will modify this texture
	qglBindTexture(GL_TEXTURE_2D, tr.renderImage->texnum);

	// Give an empty image to OpenGL ( the last "0" )
	qglTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA, glConfig.vidWidth, glConfig.vidHeight, 0,GL_RGBA, GL_UNSIGNED_BYTE, 0);

	// Poor filtering. Needed !
	qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	
	qglBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
	qglRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, glConfig.vidWidth, glConfig.vidHeight);
	qglFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrenderbuffer);

	
	qglFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tr.renderImage->texnum, 0);

	if(qglCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		ri.Printf( PRINT_ALL, "Frame buffering failed.\n" );
		return;
	}
	// Set the list of draw buffers.
	GLenum DrawBuffers[1] = {GL_NONE};
	qglDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers
	
	qglBindFramebuffer(GL_FRAMEBUFFER, fbo);
	qglViewport(0,0,glConfig.vidWidth,glConfig.vidHeight); // Render on the whole framebuffer, complete from the lower left corner to the upper right
	
	if(!tr.renderImage->texnum) {
		ri.Printf( PRINT_ALL, "Cancelling capture\n" );
		return;
	}
*/

	qglGetIntegerv(GL_PACK_ALIGNMENT, &packAlign);

	linelen = 2048 * 4;

	// Alignment stuff for glReadPixels
	padwidth = PAD(linelen, packAlign);
	padlen = padwidth - linelen;

	memcount = padwidth * 2048;

/*
	glBindTexture(GL_TEXTURE_2D, tr.renderImage->texnum);
	qglBindBuffer(GL_PIXEL_UNPACK_BUFFER, videoPBO[0]);
	qglBufferData(GL_PIXEL_UNPACK_BUFFER, memcount, 0, GL_STREAM_DRAW);
	cBuf = (GLubyte*)qglMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, memcount, 0xA);
	if(cBuf) {
		ri.CL_WriteAVIVideoFrame(cBuf, memcount);
		qglUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
	} else {
		ri.Printf( PRINT_ALL, "Cancelling capture\n" );
	}
	
	qglBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	*/
	
	
	cBuf = PADP(data, packAlign);

	qglBindBuffer(GL_PIXEL_PACK_BUFFER, videoPBO[fboIndex]);
	//qglReadPixels(0, 0, 2048, 2048, GL_RGBA, GL_UNSIGNED_BYTE, 0);

	//qglBindBuffer(GL_PIXEL_PACK_BUFFER, videoPBO[(fboIndex + 1) % 2]);
	
	//cBuf = qglMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
	qglBufferSubData(GL_PIXEL_PACK_BUFFER, 0, memcount, cBuf);
	if(cBuf) {
		ri.CL_WriteAVIVideoFrame(cBuf, memcount);
	//	qglUnmapBuffer(GL_PIXEL_PACK_BUFFER);
	} else {
		ri.Printf( PRINT_ALL, "Cancelling capture\n" );
	}
	qglBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

	GLenum DrawBuffers2[1] = {GL_NONE};
	qglDrawBuffers( 1, DrawBuffers2 );

	qglBindFramebuffer(GL_FRAMEBUFFER, 0);
	qglBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void RB_FastCaptureOld(byte *captureBuffer, byte *encodeBuffer) {
	byte				*cBuf;
	size_t				memcount, linelen;
	int				padwidth, avipadwidth, padlen, avipadlen;
	GLint packAlign;

	qglGetIntegerv(GL_PACK_ALIGNMENT, &packAlign);

	linelen = 512 * 4;

	// Alignment stuff for glReadPixels
	padwidth = PAD(linelen, packAlign);
	padlen = padwidth - linelen;
	// AVI line padding
	avipadwidth = PAD(linelen, AVI_LINE_PADDING);
	avipadlen = avipadwidth - linelen;

	cBuf = PADP(captureBuffer, packAlign);

	qglReadPixels(0, 0, 512, 512, GL_RGBA, GL_UNSIGNED_BYTE, cBuf);

	memcount = padwidth * 512;

	{
		byte *lineend;
		byte *srcptr, *destptr;

		srcptr = cBuf + memcount - padwidth;
		destptr = encodeBuffer;
		
		// swap R and B and remove line paddings
		while(srcptr >= cBuf)
		{
			lineend = srcptr + linelen;
			while(srcptr < lineend)
			{
				*destptr++ = srcptr[0];
				*destptr++ = srcptr[1];
				*destptr++ = srcptr[2];
				*destptr++ = 255;
				srcptr += 4;
			}
			
			Com_Memset(destptr, '\0', avipadlen);
			destptr += avipadlen;
			
			srcptr = srcptr - linelen - padwidth;
		}
	}
	
	ri.CL_WriteAVIVideoFrame(encodeBuffer, avipadwidth * 512);
}

/*
==================
RB_TakeVideoFrameCmd
==================
*/
const void *RB_TakeVideoFrameCmd( const void *data )
{
	const videoFrameCommand_t	*cmd;
	byte				*cBuf;
	size_t				memcount, linelen;
	int				padwidth, avipadwidth, padlen, avipadlen;
	GLint packAlign;

	// finish any 2D drawing if needed
	if(tess.numIndexes)
		RB_EndSurface();

	cmd = (const videoFrameCommand_t *)data;
	
	qglGetIntegerv(GL_PACK_ALIGNMENT, &packAlign);

	linelen = cmd->width * 3;

	// Alignment stuff for glReadPixels
	padwidth = PAD(linelen, packAlign);
	padlen = padwidth - linelen;
	// AVI line padding
	avipadwidth = PAD(linelen, AVI_LINE_PADDING);
	avipadlen = avipadwidth - linelen;

	cBuf = PADP(cmd->captureBuffer, packAlign);
		
	qglReadPixels(0, 0, cmd->width, cmd->height, GL_RGB,
		GL_UNSIGNED_BYTE, cBuf);

	memcount = padwidth * cmd->height;

	// gamma correct
	if(glConfig.deviceSupportsGamma)
		R_GammaCorrect(cBuf, memcount);

	if(cmd->motionJpeg)
	{
		memcount = ri.CL_SaveJPGToBuffer(cmd->encodeBuffer, linelen * cmd->height,
			r_aviMotionJpegQuality->integer,
			cmd->width, cmd->height, cBuf, padlen);
		ri.CL_WriteAVIVideoFrame(cmd->encodeBuffer, memcount);
	}
	else
	{
		byte *lineend, *memend;
		byte *srcptr, *destptr;
	
		srcptr = cBuf;
		destptr = cmd->encodeBuffer;
		memend = srcptr + memcount;
		
		// swap R and B and remove line paddings
		while(srcptr < memend)
		{
			lineend = srcptr + linelen;
			while(srcptr < lineend)
			{
				*destptr++ = srcptr[2];
				*destptr++ = srcptr[1];
				*destptr++ = srcptr[0];
				srcptr += 3;
			}
			
			Com_Memset(destptr, '\0', avipadlen);
			destptr += avipadlen;
			
			srcptr += padlen;
		}
		
		ri.CL_WriteAVIVideoFrame(cmd->encodeBuffer, avipadwidth * cmd->height);
	}

	return (const void *)(cmd + 1);	
}

//============================================================================

/*
** GL_SetDefaultState
*/
void GL_SetDefaultState( void )
{
	qglClearDepth( 1.0f );

	qglCullFace(GL_FRONT);

	GL_BindNullTextures();

	if (glRefConfig.framebufferObject)
		GL_BindNullFramebuffers();

	GL_TextureMode( r_textureMode->string );

	//qglShadeModel( GL_SMOOTH );
	qglDepthFunc( GL_LEQUAL );

	//
	// make sure our GL state vector is set correctly
	//
	glState.glStateBits = GLS_DEPTHTEST_DISABLE | GLS_DEPTHMASK_TRUE;
	glState.storedGlState = 0;
	glState.faceCulling = CT_TWO_SIDED;
	glState.faceCullFront = qtrue;

	GL_BindNullProgram();

	if (glRefConfig.vertexArrayObject)
		qglBindVertexArray(0);

	qglBindBuffer(GL_ARRAY_BUFFER, 0);
	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glState.currentVao = NULL;
	glState.vertexAttribsEnabled = 0;

	qglPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
	qglDepthMask( GL_TRUE );
	qglDisable( GL_DEPTH_TEST );
	qglEnable( GL_SCISSOR_TEST );
	qglDisable( GL_CULL_FACE );
	qglDisable( GL_BLEND );

	if (glRefConfig.seamlessCubeMap)
		qglEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	// GL_POLYGON_OFFSET_FILL will be glEnable()d when this is used
	qglPolygonOffset( r_offsetFactor->value, r_offsetUnits->value );

	qglClearColor( 0.0f, 0.0f, 0.0f, 1.0f );	// FIXME: get color of sky
}

/*
================
R_PrintLongString

Workaround for ri.Printf's 1024 characters buffer limit.
================
*/
void R_PrintLongString(const char *string) {
	char buffer[1024];
	const char *p;
	int size = strlen(string);

	p = string;
	while(size > 0)
	{
		Q_strncpyz(buffer, p, sizeof (buffer) );
		ri.Printf( PRINT_DEVELOPER, "%s", buffer );
		p += 1023;
		size -= 1023;
	}
}

/*
================
GfxInfo_f
================
*/
void GfxInfo_f( void ) 
{
	const char *enablestrings[] =
	{
		"disabled",
		"enabled"
	};
	const char *fsstrings[] =
	{
		"windowed",
		"fullscreen"
	};

	ri.Printf( PRINT_ALL, "\nGL_VENDOR: %s\n", glConfig.vendor_string );
	ri.Printf( PRINT_ALL, "GL_RENDERER: %s\n", glConfig.renderer_string );
	ri.Printf( PRINT_ALL, "GL_VERSION: %s\n", glConfig.version_string );
	ri.Printf( PRINT_ALL, "GL_EXTENSIONS: " );
	if ( qglGetStringi )
	{
		GLint numExtensions;
		int i;

		qglGetIntegerv( GL_NUM_EXTENSIONS, &numExtensions );
		for ( i = 0; i < numExtensions; i++ )
		{
			ri.Printf( PRINT_ALL, "%s ", qglGetStringi( GL_EXTENSIONS, i ) );
		}
	}
	else
	{
		R_PrintLongString( glConfig.extensions_string );
	}
	ri.Printf( PRINT_ALL, "\n" );
	ri.Printf( PRINT_ALL, "GL_MAX_TEXTURE_SIZE: %d\n", glConfig.maxTextureSize );
	ri.Printf( PRINT_ALL, "GL_MAX_TEXTURE_IMAGE_UNITS: %d\n", glConfig.numTextureUnits );
	ri.Printf( PRINT_ALL, "\nPIXELFORMAT: color(%d-bits) Z(%d-bit) stencil(%d-bits)\n", glConfig.colorBits, glConfig.depthBits, glConfig.stencilBits );
	ri.Printf( PRINT_ALL, "MODE: %d, %d x %d %s hz:", ri.Cvar_VariableIntegerValue( "r_mode" ), glConfig.vidWidth, glConfig.vidHeight, fsstrings[ glConfig.isFullscreen != 0 ] );
	if ( glConfig.displayFrequency )
	{
		ri.Printf( PRINT_ALL, "%d\n", glConfig.displayFrequency );
	}
	else
	{
		ri.Printf( PRINT_ALL, "N/A\n" );
	}
	if ( glConfig.deviceSupportsGamma )
	{
		ri.Printf( PRINT_ALL, "GAMMA: hardware w/ %d overbright bits\n", tr.overbrightBits );
	}
	else
	{
		ri.Printf( PRINT_ALL, "GAMMA: software w/ %d overbright bits\n", tr.overbrightBits );
	}

	ri.Printf( PRINT_ALL, "texturemode: %s\n", r_textureMode->string );
	ri.Printf( PRINT_ALL, "picmip: %d\n", r_picmip->integer );
	ri.Printf( PRINT_ALL, "texture bits: %d\n", r_texturebits->integer );
	ri.Printf( PRINT_ALL, "texenv add: %s\n", enablestrings[glConfig.textureEnvAddAvailable != 0] );
	ri.Printf( PRINT_ALL, "compressed textures: %s\n", enablestrings[glConfig.textureCompression!=TC_NONE] );
	
	if ( r_finish->integer ) {
		ri.Printf( PRINT_ALL, "Forcing glFinish\n" );
	}
}


/*
================
GfxMemInfo_f
================
*/
void GfxMemInfo_f( void ) 
{
	switch (glRefConfig.memInfo)
	{
		case MI_NONE:
		{
			ri.Printf(PRINT_ALL, "No extension found for GPU memory info.\n");
		}
		break;
		case MI_NVX:
		{
			int value;

			qglGetIntegerv(GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX, &value);
			ri.Printf(PRINT_ALL, "GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX: %ikb\n", value);

			qglGetIntegerv(GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, &value);
			ri.Printf(PRINT_ALL, "GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX: %ikb\n", value);

			qglGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &value);
			ri.Printf(PRINT_ALL, "GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX: %ikb\n", value);

			qglGetIntegerv(GL_GPU_MEMORY_INFO_EVICTION_COUNT_NVX, &value);
			ri.Printf(PRINT_ALL, "GPU_MEMORY_INFO_EVICTION_COUNT_NVX: %i\n", value);

			qglGetIntegerv(GL_GPU_MEMORY_INFO_EVICTED_MEMORY_NVX, &value);
			ri.Printf(PRINT_ALL, "GPU_MEMORY_INFO_EVICTED_MEMORY_NVX: %ikb\n", value);
		}
		break;
		case MI_ATI:
		{
			// GL_ATI_meminfo
			int value[4];

			qglGetIntegerv(GL_VBO_FREE_MEMORY_ATI, &value[0]);
			ri.Printf(PRINT_ALL, "VBO_FREE_MEMORY_ATI: %ikb total %ikb largest aux: %ikb total %ikb largest\n", value[0], value[1], value[2], value[3]);

			qglGetIntegerv(GL_TEXTURE_FREE_MEMORY_ATI, &value[0]);
			ri.Printf(PRINT_ALL, "TEXTURE_FREE_MEMORY_ATI: %ikb total %ikb largest aux: %ikb total %ikb largest\n", value[0], value[1], value[2], value[3]);

			qglGetIntegerv(GL_RENDERBUFFER_FREE_MEMORY_ATI, &value[0]);
			ri.Printf(PRINT_ALL, "RENDERBUFFER_FREE_MEMORY_ATI: %ikb total %ikb largest aux: %ikb total %ikb largest\n", value[0], value[1], value[2], value[3]);
		}
		break;
	}
}


/*
===============
R_Register
===============
*/
void R_Register( void ) 
{
	//
	// latched and archived variables
	//
	r_allowExtensions = ri.Cvar_Get( "r_allowExtensions", "1", CVAR_ARCHIVE | CVAR_LATCH );
	ri.Cvar_SetDescription(r_allowExtensions, "Use all of the OpenGL extensions the card is capable of\nDefault: 1");
	r_ext_compressed_textures = ri.Cvar_Get( "r_ext_compressed_textures", "0", CVAR_ARCHIVE | CVAR_LATCH );
	ri.Cvar_SetDescription(r_ext_compressed_textures, "Compress textures as they are loaded\nDefault: 1");
	r_ext_multitexture = ri.Cvar_Get( "r_ext_multitexture", "1", CVAR_ARCHIVE | CVAR_LATCH );
	ri.Cvar_SetDescription(r_ext_multitexture, "Enable multitexture, not used\nDefault: 1");
	r_ext_compiled_vertex_array = ri.Cvar_Get( "r_ext_compiled_vertex_array", "1", CVAR_ARCHIVE | CVAR_LATCH);
	ri.Cvar_SetDescription(r_ext_compiled_vertex_array, "Enable vertex arrays\nDefault: 1");
	r_ext_texture_env_add = ri.Cvar_Get( "r_ext_texture_env_add", "1", CVAR_ARCHIVE | CVAR_LATCH);
	ri.Cvar_SetDescription(r_ext_texture_env_add, "Enable texture environment\nDefault: 1");

	r_ext_framebuffer_object = ri.Cvar_Get( "r_ext_framebuffer_object", "1", CVAR_ARCHIVE | CVAR_LATCH);
	ri.Cvar_SetDescription(r_ext_framebuffer_object, "Enable framebuffer objects (FBOs) for buffered rendering\nDefault: 1");
	r_ext_texture_float = ri.Cvar_Get( "r_ext_texture_float", "1", CVAR_ARCHIVE | CVAR_LATCH);
	ri.Cvar_SetDescription(r_ext_texture_float, "Enable texture floats\nDefault: 1");
	r_ext_framebuffer_multisample = ri.Cvar_Get( "r_ext_framebuffer_multisample", "0", CVAR_ARCHIVE | CVAR_LATCH);
	ri.Cvar_SetDescription(r_ext_framebuffer_multisample, "Enable framebuffer multisampling\nDefault: 0");
	r_arb_seamless_cube_map = ri.Cvar_Get( "r_arb_seamless_cube_map", "0", CVAR_ARCHIVE | CVAR_LATCH);
	ri.Cvar_SetDescription(r_arb_seamless_cube_map, "Enable seamless cube mapping\nDefault: 0");
	r_arb_vertex_array_object = ri.Cvar_Get( "r_arb_vertex_array_object", "1", CVAR_ARCHIVE | CVAR_LATCH);
	ri.Cvar_SetDescription(r_arb_vertex_array_object, "Enable ARB vertex array objects\nDefault: 1");
	r_ext_direct_state_access = ri.Cvar_Get("r_ext_direct_state_access", "1", CVAR_ARCHIVE | CVAR_LATCH);
	ri.Cvar_SetDescription(r_ext_direct_state_access, "Enable direct state access\nDefault: 1");

	r_ext_texture_filter_anisotropic = ri.Cvar_Get( "r_ext_texture_filter_anisotropic",
			"0", CVAR_ARCHIVE | CVAR_LATCH );
	ri.Cvar_SetDescription(r_ext_texture_filter_anisotropic, "Enable texture anisotropic filtering\nDefault: 0");
	r_ext_max_anisotropy = ri.Cvar_Get( "r_ext_max_anisotropy", "2", CVAR_ARCHIVE | CVAR_LATCH );
	ri.Cvar_SetDescription(r_ext_max_anisotropy, "Enable max anisotropy\nDefault: 2");

	r_picmip = ri.Cvar_Get ("r_picmip", "1", CVAR_ARCHIVE | CVAR_LATCH );
	ri.Cvar_CheckRange( r_picmip, "0", "16", CV_INTEGER );
	ri.Cvar_SetDescription(r_picmip, "Set maximum texture size\n0 - best quality\n4 - fastest\nDefault: 1");
	r_roundImagesDown = ri.Cvar_Get ("r_roundImagesDown", "1", CVAR_ARCHIVE | CVAR_LATCH );
	ri.Cvar_SetDescription(r_roundImagesDown, "Set rounding down amount\nDefault: 1");
	r_colorMipLevels = ri.Cvar_Get ("r_colorMipLevels", "0", CVAR_LATCH );
	ri.Cvar_CheckRange( r_colorMipLevels, "0", "16", CV_INTEGER );
	ri.Cvar_SetDescription(r_colorMipLevels, "Enable texture visualizations for debugging\nDefault: 0");
	r_detailTextures = ri.Cvar_Get( "r_detailtextures", "1", CVAR_ARCHIVE | CVAR_LATCH );
	ri.Cvar_SetDescription(r_detailTextures, "Enable detailed textures\nDefault: 1");
	r_texturebits = ri.Cvar_Get( "r_texturebits", "0", CVAR_ARCHIVE | CVAR_LATCH );
	ri.Cvar_SetDescription(r_texturebits, "Set the number of bits used for a texture\nDefault: 0");
	r_stencilbits = ri.Cvar_Get( "r_stencilbits", "8", CVAR_ARCHIVE | CVAR_LATCH );
	ri.Cvar_SetDescription(r_stencilbits, "Set the number of stencil bits, 0, 8, 16\nDefault: 8");
	r_ext_multisample = ri.Cvar_Get( "r_ext_multisample", "0", CVAR_ARCHIVE | CVAR_LATCH );
	ri.Cvar_CheckRange( r_ext_multisample, "0", "4", CV_INTEGER );
	ri.Cvar_SetDescription(r_ext_multisample, "Enable frame multisampling, 0 - 4\nDefault: 0");
	r_overBrightBits = ri.Cvar_Get ("r_overBrightBits", "1", CVAR_ARCHIVE | CVAR_LATCH );
	ri.Cvar_SetDescription(r_overBrightBits, "Set intensity level of lights reflected from textures\nDefault: 1");
	r_ignorehwgamma = ri.Cvar_Get( "r_ignorehwgamma", "0", CVAR_ARCHIVE | CVAR_LATCH);
	ri.Cvar_SetDescription(r_ignorehwgamma, "Toggle the use of video driver gamma correction\nDefault: 0");
	r_simpleMipMaps = ri.Cvar_Get( "r_simpleMipMaps", "1", CVAR_ARCHIVE | CVAR_LATCH );
	ri.Cvar_SetDescription(r_simpleMipMaps, "Toggle the use of simple mip mapping for slower machines\nDefault: 1");
	r_vertexLight = ri.Cvar_Get( "r_vertexLight", "0", CVAR_ARCHIVE | CVAR_LATCH );
	ri.Cvar_SetDescription(r_vertexLight, "Enable vertex lighting (faster, lower quality than lightmap) removes lightmaps, forces every shader to only use a single rendering pass\nDefault: 0");
	r_subdivisions = ri.Cvar_Get ("r_subdivisions", "4", CVAR_ARCHIVE | CVAR_LATCH);
	ri.Cvar_SetDescription(r_subdivisions, "Set maximum level of detail\nDefault: 4");
	r_greyscale = ri.Cvar_Get("r_greyscale", "0", CVAR_ARCHIVE | CVAR_LATCH);
	ri.Cvar_CheckRange( r_greyscale, "0", "1", CV_FLOAT );
	ri.Cvar_SetDescription(r_greyscale, "Enable grayscale effect where all images are converted to grayscale\nDefault: 0");
	r_paletteMode = ri.Cvar_Get("r_paletteMode", "0", CVAR_ARCHIVE | CVAR_LATCH);
	r_seeThroughWalls = ri.Cvar_Get("r_seeThroughWalls", "0", CVAR_ARCHIVE | CVAR_LATCH);

	r_externalGLSL = ri.Cvar_Get( "r_externalGLSL", "0", CVAR_LATCH );
	ri.Cvar_SetDescription(r_externalGLSL, "Support loading GLSL files externally, Mods can supply their own rendering\nDefault: 0");

	r_hdr = ri.Cvar_Get( "r_hdr", "1", CVAR_ARCHIVE | CVAR_LATCH );
	ri.Cvar_SetDescription(r_hdr, "Enable High definition light maps\nDefault: 1");
	r_floatLightmap = ri.Cvar_Get( "r_floatLightmap", "0", CVAR_ARCHIVE | CVAR_LATCH );
	ri.Cvar_SetDescription(r_floatLightmap, "Allow lightmaps to use floats for alignment, not used\nDefault: 0");
	r_postProcess = ri.Cvar_Get( "r_postProcess", "1", CVAR_ARCHIVE );
	ri.Cvar_SetDescription(r_postProcess, "Enable post processing effects such as motion blur\nDefault: 1");

	r_toneMap = ri.Cvar_Get( "r_toneMap", "1", CVAR_ARCHIVE );
	ri.Cvar_SetDescription(r_toneMap, "Enable tone mapping, like in photographer, colors are evened out based on the generated scene, requires postprocessing\nDefault: 1");
	r_forceToneMap = ri.Cvar_Get( "r_forceToneMap", "0", CVAR_CHEAT );
	ri.Cvar_SetDescription(r_forceToneMap, "Force tone mapping on every frame using custom values\nDefault: 0");
	r_forceToneMapMin = ri.Cvar_Get( "r_forceToneMapMin", "-8.0", CVAR_CHEAT );
	ri.Cvar_SetDescription(r_forceToneMapMin, "Set the tone map minimum darkness\nDefault: -8.0");
	r_forceToneMapAvg = ri.Cvar_Get( "r_forceToneMapAvg", "-2.0", CVAR_CHEAT );
	ri.Cvar_SetDescription(r_forceToneMapAvg, "Set the tone map average brightness/darkness\nDefault: -2.0");
	r_forceToneMapMax = ri.Cvar_Get( "r_forceToneMapMax", "0.0", CVAR_CHEAT );
	ri.Cvar_SetDescription(r_forceToneMapMax, "Set the max tone map brightness\nDefault: 0.0");

	r_autoExposure = ri.Cvar_Get( "r_autoExposure", "1", CVAR_ARCHIVE );
	ri.Cvar_SetDescription(r_autoExposure, "Enable tone map auto exposure, just like in photography, affects how lighting is rendered\nDefault: 1");
	r_forceAutoExposure = ri.Cvar_Get( "r_forceAutoExposure", "0", CVAR_CHEAT );
	ri.Cvar_SetDescription(r_forceAutoExposure, "Force auto exposure values in post processing\nDefault: 0");
	r_forceAutoExposureMin = ri.Cvar_Get( "r_forceAutoExposureMin", "-2.0", CVAR_CHEAT );
	ri.Cvar_SetDescription(r_forceAutoExposureMin, "Set the minimum for exposure\nDefault: -2.0");
	r_forceAutoExposureMax = ri.Cvar_Get( "r_forceAutoExposureMax", "2.0", CVAR_CHEAT );
	ri.Cvar_SetDescription(r_forceAutoExposureMax, "Set the maximum for exposure\nDefault: 2.0");

	r_cameraExposure = ri.Cvar_Get( "r_cameraExposure", "1", CVAR_CHEAT );
	ri.Cvar_SetDescription(r_cameraExposure, "Enable camera exposure when looking through portals\nDefault: 1");

	r_depthPrepass = ri.Cvar_Get( "r_depthPrepass", "1", CVAR_ARCHIVE );
	ri.Cvar_SetDescription(r_depthPrepass, "Enable prepass depth rendering\nDefault 1");
	r_ssao = ri.Cvar_Get( "r_ssao", "0", CVAR_LATCH | CVAR_ARCHIVE );
	ri.Cvar_SetDescription(r_ssao, "Enable screen space ambient occlusion for more realistic lighting\nDefault: 0");

	r_normalMapping = ri.Cvar_Get( "r_normalMapping", "1", CVAR_ARCHIVE | CVAR_LATCH );
	ri.Cvar_SetDescription(r_normalMapping, "Enable normal mapping for lighting\nDefault: 1");
	r_specularMapping = ri.Cvar_Get( "r_specularMapping", "1", CVAR_ARCHIVE | CVAR_LATCH );
	ri.Cvar_SetDescription(r_specularMapping, "Enable specular mapping for lighting\nDefault: 1");
	r_deluxeMapping = ri.Cvar_Get( "r_deluxeMapping", "1", CVAR_ARCHIVE | CVAR_LATCH );
	ri.Cvar_SetDescription(r_deluxeMapping, "Enable deluxe mapping for lighting\nDefault: 1");
	r_parallaxMapping = ri.Cvar_Get( "r_parallaxMapping", "0", CVAR_ARCHIVE | CVAR_LATCH );
	ri.Cvar_SetDescription(r_parallaxMapping, "Enable parallax mapping for lighting\nDefault: 0");
	r_parallaxMapShadows = ri.Cvar_Get( "r_parallaxMapShadows", "0", CVAR_ARCHIVE | CVAR_LATCH );
	ri.Cvar_SetDescription(r_parallaxMapShadows, "Enable parallax mapping for shadows\nDefault: 0");
	r_cubeMapping = ri.Cvar_Get( "r_cubeMapping", "0", CVAR_ARCHIVE | CVAR_LATCH );
	ri.Cvar_SetDescription(r_cubeMapping, "Enable cube mapping for lighting\nDefault: 0");
	r_cubemapSize = ri.Cvar_Get( "r_cubemapSize", "128", CVAR_ARCHIVE | CVAR_LATCH );
	ri.Cvar_SetDescription(r_cubemapSize, "Set the cube mapping size\nDefault: 128");
	r_deluxeSpecular = ri.Cvar_Get("r_deluxeSpecular", "0.3", CVAR_ARCHIVE | CVAR_LATCH);
	ri.Cvar_SetDescription(r_deluxeSpecular, "Enable delux specular mapping for lighting\nDefault: 0.3");
	r_pbr = ri.Cvar_Get("r_pbr", "0", CVAR_ARCHIVE | CVAR_LATCH);
	ri.Cvar_SetDescription(r_pbr, "Enable physics based renderering\nDefault: 0");
	r_baseNormalX = ri.Cvar_Get( "r_baseNormalX", "1.0", CVAR_ARCHIVE | CVAR_LATCH );
	//ri.Cvar_SetDescription();
	r_baseNormalY = ri.Cvar_Get( "r_baseNormalY", "1.0", CVAR_ARCHIVE | CVAR_LATCH );
	//ri.Cvar_SetDescription();
	r_baseParallax = ri.Cvar_Get( "r_baseParallax", "0.05", CVAR_ARCHIVE | CVAR_LATCH );
	//ri.Cvar_SetDescription();
	r_baseSpecular = ri.Cvar_Get( "r_baseSpecular", "0.04", CVAR_ARCHIVE | CVAR_LATCH );
	//ri.Cvar_SetDescription();
	r_baseGloss = ri.Cvar_Get( "r_baseGloss", "0.3", CVAR_ARCHIVE | CVAR_LATCH );
	//ri.Cvar_SetDescription();
	r_glossType = ri.Cvar_Get("r_glossType", "1", CVAR_ARCHIVE | CVAR_LATCH);
	//ri.Cvar_SetDescription();
	r_dlightMode = ri.Cvar_Get( "r_dlightMode", "0", CVAR_ARCHIVE | CVAR_LATCH );
	//ri.Cvar_SetDescription();
	r_pshadowDist = ri.Cvar_Get( "r_pshadowDist", "128", CVAR_ARCHIVE );
	//ri.Cvar_SetDescription();
#ifdef USE_LAZY_MEMORY
	// turn off lightmap merge so they can be updated every time the world loads
	r_mergeLightmaps = ri.Cvar_Get( "r_mergeLightmaps", "0", CVAR_ARCHIVE | CVAR_LATCH );
	ri.Cvar_CheckRange( r_mergeLightmaps, "0", "0", CV_INTEGER );
#else
	r_mergeLightmaps = ri.Cvar_Get( "r_mergeLightmaps", "1", CVAR_ARCHIVE | CVAR_LATCH );
#endif
	//ri.Cvar_SetDescription();
	r_imageUpsample = ri.Cvar_Get( "r_imageUpsample", "0", CVAR_ARCHIVE | CVAR_LATCH );
	//ri.Cvar_SetDescription();
	r_imageUpsampleMaxSize = ri.Cvar_Get( "r_imageUpsampleMaxSize", "1024", CVAR_ARCHIVE | CVAR_LATCH );
	//ri.Cvar_SetDescription();
	r_imageUpsampleType = ri.Cvar_Get( "r_imageUpsampleType", "1", CVAR_ARCHIVE | CVAR_LATCH );
	//ri.Cvar_SetDescription();
	r_genNormalMaps = ri.Cvar_Get( "r_genNormalMaps", "0", CVAR_ARCHIVE | CVAR_LATCH );
	//ri.Cvar_SetDescription();

	r_forceSun = ri.Cvar_Get( "r_forceSun", "0", CVAR_CHEAT );
	//ri.Cvar_SetDescription();
	r_forceSunLightScale = ri.Cvar_Get( "r_forceSunLightScale", "1.0", CVAR_CHEAT );
	//ri.Cvar_SetDescription();
	r_forceSunAmbientScale = ri.Cvar_Get( "r_forceSunAmbientScale", "0.5", CVAR_CHEAT );
	//ri.Cvar_SetDescription();
	r_drawSunRays = ri.Cvar_Get( "r_drawSunRays", "0", CVAR_ARCHIVE | CVAR_LATCH );
	//ri.Cvar_SetDescription();
	r_sunlightMode = ri.Cvar_Get( "r_sunlightMode", "1", CVAR_ARCHIVE | CVAR_LATCH );
	//ri.Cvar_SetDescription();

	r_sunShadows = ri.Cvar_Get( "r_sunShadows", "1", CVAR_ARCHIVE | CVAR_LATCH );
	//ri.Cvar_SetDescription();
	r_shadowFilter = ri.Cvar_Get( "r_shadowFilter", "1", CVAR_ARCHIVE | CVAR_LATCH );
	//ri.Cvar_SetDescription();
	r_shadowBlur = ri.Cvar_Get("r_shadowBlur", "0", CVAR_ARCHIVE | CVAR_LATCH);
	//ri.Cvar_SetDescription();
	r_shadowMapSize = ri.Cvar_Get("r_shadowMapSize", "1024", CVAR_ARCHIVE | CVAR_LATCH);
	//ri.Cvar_SetDescription();
	r_shadowCascadeZNear = ri.Cvar_Get( "r_shadowCascadeZNear", "8", CVAR_ARCHIVE | CVAR_LATCH );
	//ri.Cvar_SetDescription();
	r_shadowCascadeZFar = ri.Cvar_Get( "r_shadowCascadeZFar", "1024", CVAR_ARCHIVE | CVAR_LATCH );
	//ri.Cvar_SetDescription();
	r_shadowCascadeZBias = ri.Cvar_Get( "r_shadowCascadeZBias", "0", CVAR_ARCHIVE | CVAR_LATCH );
	//ri.Cvar_SetDescription();
	r_ignoreDstAlpha = ri.Cvar_Get( "r_ignoreDstAlpha", "1", CVAR_ARCHIVE | CVAR_LATCH );
	//ri.Cvar_SetDescription();

	//
	// temporary latched variables that can only change over a restart
	//
	r_fullbright = ri.Cvar_Get ("r_fullbright", "0", CVAR_LATCH|CVAR_CHEAT );
	ri.Cvar_SetDescription(r_fullbright, "Toggle textures to full brightness level\nDefault: 0");
	r_mapOverBrightBits = ri.Cvar_Get ("r_mapOverBrightBits", "2", CVAR_LATCH );
	ri.Cvar_SetDescription(r_mapOverBrightBits, "Set intensity level of lights reflected from textures\nDefault: 2");
	r_intensity = ri.Cvar_Get ("r_intensity", "1", CVAR_LATCH );
	ri.Cvar_SetDescription(r_intensity, "Increase brightness of texture colors\nDefault: 1");
	r_singleShader = ri.Cvar_Get ("r_singleShader", "0", CVAR_CHEAT | CVAR_LATCH );
	ri.Cvar_SetDescription(r_singleShader, "Toggles use of 1 default shader for all objects\nDefault: 0");

	//
	// archived variables that can change at any time
	//
	r_lodCurveError = ri.Cvar_Get( "r_lodCurveError", "250", CVAR_ARCHIVE );
	ri.Cvar_CheckRange( r_lodCurveError, "-1", "8192", CV_FLOAT );
	ri.Cvar_SetDescription( r_lodCurveError, "Level of detail error on curved surface grids." );
	r_lodCurveError = ri.Cvar_Get( "r_lodCurveError", "250", CVAR_ARCHIVE|CVAR_CHEAT );
	ri.Cvar_SetDescription(r_lodCurveError, "Level of detail setting if set to 10000, don't drop curve rows for a long time\nDefault: 250");
	r_lodbias = ri.Cvar_Get( "r_lodbias", "0", CVAR_ARCHIVE );
	ri.Cvar_SetDescription(r_lodbias, "Change the geometric level of detail, 0 - high detail, 4 - low detail\nDefault: 0");
	r_flares = ri.Cvar_Get ("r_flares", "0", CVAR_ARCHIVE );
	ri.Cvar_SetDescription(r_flares, "Toggle projectile flare and lighting effect\nDefault: 0");
	r_znear = ri.Cvar_Get( "r_znear", "4", CVAR_CHEAT );
	ri.Cvar_CheckRange( r_znear, "0.001", "200", CV_FLOAT );
	ri.Cvar_CheckRange( r_znear, "0.001", NULL, CV_FLOAT );
	ri.Cvar_SetDescription(r_znear, "Set how close objects can be to the player before they're clipped out of the scene, so you can't see your nose or shoulders\nDefault: 0.001");
	r_zfar = ri.Cvar_Get( "r_zfar", "0", CVAR_ARCHIVE|CVAR_CHEAT );
	ri.Cvar_CheckRange( r_zfar, "0", NULL, CV_FLOAT );
	ri.Cvar_SetDescription( r_zfar, "Set how far objects are before they are clipped out, usually automatically calculated based on map size, 0 - infinity, 2048 - used for menus\nDefault: 0 infinity" );
	r_zproj = ri.Cvar_Get( "r_zproj", "64", CVAR_ARCHIVE );
	//ri.Cvar_SetDescription();
	r_stereoSeparation = ri.Cvar_Get( "r_stereoSeparation", "64", CVAR_ARCHIVE );
	ri.Cvar_SetDescription(r_stereoSeparation, "Set the distance between stereo frame renders, as if your eyes are separated\nDefault: 64");
	r_ignoreGLErrors = ri.Cvar_Get( "r_ignoreGLErrors", "1", CVAR_ARCHIVE );
	ri.Cvar_SetDescription(r_ignoreGLErrors, "Ignores OpenGL errors that occur\nDefault: 1");
	r_fastsky = ri.Cvar_Get( "r_fastsky", "0", CVAR_ARCHIVE );
	ri.Cvar_SetDescription(r_fastsky, "Toggle fast rendering of sky if set to 1, also disables portals\nDefault: 0");
	r_drawSun = ri.Cvar_Get( "r_drawSun", "0", CVAR_ARCHIVE );
	ri.Cvar_SetDescription(r_drawSun, "Toggle rendering of sunlight in lighting effects\nDefault: 0");
	r_dynamiclight = ri.Cvar_Get( "r_dynamiclight", "1", CVAR_ARCHIVE );
	ri.Cvar_SetDescription(r_dynamiclight, "Toggle dynamic lighting, where all visuals are darker or brighter depending on lightmap\nDefault: 1");
	r_dlightBacks = ri.Cvar_Get( "r_dlightBacks", "1", CVAR_ARCHIVE );
	ri.Cvar_SetDescription(r_dlightBacks, "Brighter areas are changed more by dlights than dark areas\nDefault: 1");
	r_finish = ri.Cvar_Get ("r_finish", "0", CVAR_ARCHIVE);
	ri.Cvar_SetDescription(r_finish, "Toggle synchronization of rendered frames\nDefault: 0");
	r_textureMode = ri.Cvar_Get( "r_textureMode", "GL_LINEAR_MIPMAP_NEAREST", CVAR_ARCHIVE );
	ri.Cvar_SetDescription(r_textureMode, "Select texture mode\nDefault: GL_LINEAR_MIPMAP_NEAREST");
	r_gamma = ri.Cvar_Get( "r_gamma", "1", CVAR_ARCHIVE );
	ri.Cvar_CheckRange( r_gamma, "0.5", "3", CV_FLOAT );
	ri.Cvar_SetDescription(r_gamma, "Set gamma correction, 0.5 - 3\nDefault: 1");
	r_facePlaneCull = ri.Cvar_Get ("r_facePlaneCull", "1", CVAR_ARCHIVE );
	ri.Cvar_SetDescription(r_facePlaneCull, "Toggle culling of brush faces not in view, 0 affects performance\nDefault: 1");

	r_railWidth = ri.Cvar_Get( "r_railWidth", "16", CVAR_ARCHIVE );
	ri.Cvar_SetDescription(r_railWidth, "Set width of the rail trail\nDefault: 16");
	r_railCoreWidth = ri.Cvar_Get( "r_railCoreWidth", "6", CVAR_ARCHIVE );
	ri.Cvar_SetDescription(r_railCoreWidth, "Set size of the rail trail's core\nDefault: 6");
	r_railSegmentLength = ri.Cvar_Get( "r_railSegmentLength", "32", CVAR_ARCHIVE );
	ri.Cvar_SetDescription(r_railSegmentLength, "Set distance between rail sun bursts\nDefault: 32");

	r_ambientScale = ri.Cvar_Get( "r_ambientScale", "0.6", CVAR_CHEAT );
	ri.Cvar_SetDescription(r_ambientScale, "Set the scale or intensity of ambient light\nDefault: 0.6");
	r_directedScale = ri.Cvar_Get( "r_directedScale", "1", CVAR_CHEAT );
	ri.Cvar_SetDescription(r_directedScale, "Set scale/intensity of light shinning directly upon objects\nDefault: 1");

	r_anaglyphMode = ri.Cvar_Get("r_anaglyphMode", "0", CVAR_ARCHIVE);
	ri.Cvar_SetDescription(r_anaglyphMode, "Enable anaglyph mode when using the red and blue lense 3D glasses\nDefault: 0");

	//
	// temporary variables that can change at any time
	//
	r_showImages = ri.Cvar_Get( "r_showImages", "0", CVAR_TEMP );
	ri.Cvar_SetDescription(r_showImages, "Toggle displaying a collage of all image files when set to 1\nDefault: 0");

	r_debugLight = ri.Cvar_Get( "r_debuglight", "0", CVAR_TEMP );
	ri.Cvar_SetDescription(r_debugLight, "Toggle debugging of lighting effects\nDefault: 0");
	r_debugSort = ri.Cvar_Get( "r_debugSort", "0", CVAR_CHEAT );
	ri.Cvar_SetDescription(r_debugSort, "Toggle debugging of sorting of polygons for depth\nDefault: 0");
	r_printShaders = ri.Cvar_Get( "r_printShaders", "0", 0 );
	ri.Cvar_SetDescription(r_printShaders, "Toggle the printing on console of the number of shaders\nDefault: 0");
	r_saveFontData = ri.Cvar_Get( "r_saveFontData", "0", 0 );
	ri.Cvar_SetDescription(r_saveFontData, "Enable saving of font image data when they are loaded in game, for developers\nDefault: 0");

	r_nocurves = ri.Cvar_Get ("r_nocurves", "0", CVAR_CHEAT );
	ri.Cvar_SetDescription(r_nocurves, "Map diagnostic command toggle the use of curved geometry\nDefault: 0");
	r_drawworld = ri.Cvar_Get ("r_drawworld", "0", CVAR_CHEAT );
	ri.Cvar_SetDescription(r_drawworld, "Toggle rendering of map architecture\nDefault: 1");
	r_lightmap = ri.Cvar_Get ("r_lightmap", "0", 0 );
	ri.Cvar_SetDescription(r_lightmap, "Toggle entire map to full brightness level all textures become blurred with light\nDefault: 0");
	r_portalOnly = ri.Cvar_Get ("r_portalOnly", "0", CVAR_CHEAT );
	ri.Cvar_SetDescription(r_portalOnly, "When set to 1 turns off stencil buffering for portals, this allows you to see the entire portal before it's clipped\nDefault: 0");

	r_flareSize = ri.Cvar_Get ("r_flareSize", "40", CVAR_CHEAT);
	ri.Cvar_SetDescription(r_flareSize, "Set the size of flares\nDefault: 40");
	r_flareFade = ri.Cvar_Get ("r_flareFade", "7", CVAR_CHEAT);
	ri.Cvar_SetDescription(r_flareFade, "Set scale of fading of flares in relation to distance\nDefault: 7");
	r_flareCoeff = ri.Cvar_Get ("r_flareCoeff", FLARE_STDCOEFF, CVAR_CHEAT);
	ri.Cvar_SetDescription(r_flareCoeff, "Set the flare coefficient\nDefault: " XSTRING(FLARE_STDCOEFF));

	r_skipBackEnd = ri.Cvar_Get ("r_skipBackEnd", "0", CVAR_CHEAT);
	ri.Cvar_SetDescription(r_skipBackEnd, "Toggle the skipping of the backend video buffer\nDefault: 0");

	r_measureOverdraw = ri.Cvar_Get( "r_measureOverdraw", "0", CVAR_CHEAT );
	ri.Cvar_SetDescription(r_measureOverdraw, "Measure overdraw, when the same pixel is written to more than once when rendering a scene\nDefault: 0");
	r_lodscale = ri.Cvar_Get( "r_lodscale", "5", CVAR_CHEAT );
	ri.Cvar_SetDescription(r_lodscale, "Set scale for level of detail adjustment\nDefault: 5");
#ifdef USE_LAZY_LOAD
	r_lazyLoad = ri.Cvar_Get( "cl_lazyLoad", "0", 0 );
	ri.Cvar_Get("r_loadingModel", "", CVAR_TEMP);
	ri.Cvar_Get("r_loadingShader", "", CVAR_TEMP);
#endif
	r_norefresh = ri.Cvar_Get ("r_norefresh", "0", CVAR_CHEAT);
	ri.Cvar_SetDescription(r_norefresh, "Toggle the refreshing of the rendered display, for debugging\nDefault: 0");
	r_drawentities = ri.Cvar_Get ("r_drawentities", "1", CVAR_CHEAT );
	ri.Cvar_SetDescription(r_drawentities, "Toggle display of brush entities\nDefault: 1");
	r_ignore = ri.Cvar_Get( "r_ignore", "1", CVAR_CHEAT );
	ri.Cvar_SetDescription(r_ignore, "Ignores hardware driver settings in favor of variable settings\nDefault: 1");
	r_nocull = ri.Cvar_Get ("r_nocull", "0", CVAR_CHEAT);
	ri.Cvar_SetDescription(r_nocull, "Toggle rendering of hidden objects\nDefault: 0");
	r_novis = ri.Cvar_Get ("r_novis", "0", CVAR_CHEAT);
	ri.Cvar_SetDescription(r_novis, "Disable VIS tables that hold information about which areas should be displayed, draw all polygons\nDefault: 0");
	r_showcluster = ri.Cvar_Get ("r_showcluster", "0", CVAR_CHEAT);
	ri.Cvar_SetDescription(r_showcluster, "Toggle the display of clusters by number as the player enters the area\nDefault: 0");
	r_speeds = ri.Cvar_Get ("r_speeds", "0", CVAR_CHEAT);
	ri.Cvar_SetDescription(r_speeds, "Show rendering info e.g. how many triangles are drawn added, timing info\nDefault: 0");
	r_debugSurface = ri.Cvar_Get ("r_debugSurface", "0", CVAR_CHEAT);
	ri.Cvar_SetDescription(r_debugSurface, "Enable drawing of surface debug information on the polygon surface\nDefault: 0");
	r_nobind = ri.Cvar_Get ("r_nobind", "0", CVAR_CHEAT);
	ri.Cvar_SetDescription(r_nobind, "Toggle the binding of textures to triangles\nDefault: 0");
	r_showtris = ri.Cvar_Get ("r_showtris", "0", CVAR_CHEAT);
	ri.Cvar_SetDescription(r_showtris, "Enable map diagnostics and show triangles around each polygon\nDefault: 0");
	r_showsky = ri.Cvar_Get ("r_showsky", "0", CVAR_CHEAT);
	ri.Cvar_SetDescription(r_showsky, "Enable rendering sky in front of other objects\nDefault: 0");
	r_shownormals = ri.Cvar_Get ("r_shownormals", "0", CVAR_CHEAT);
	ri.Cvar_SetDescription(r_shownormals, "Toggle the drawing of short lines indicating brush and entity polygon vertices, useful when debugging model lighting\nDefault: 0");
	r_clear = ri.Cvar_Get ("r_clear", "0", CVAR_CHEAT);
	ri.Cvar_SetDescription(r_clear, "Toggle the clearing of the screen between frames\nDefault: 0");
	r_offsetFactor = ri.Cvar_Get( "r_offsetfactor", "-1", CVAR_CHEAT );
	ri.Cvar_SetDescription(r_offsetFactor, "Control the polygon offset factor, when you see lines appearing in decals, or they seem to flick on and off\nDefault: -1");
	r_offsetUnits = ri.Cvar_Get( "r_offsetunits", "-2", CVAR_CHEAT );
	ri.Cvar_SetDescription(r_offsetUnits, "Control the polygon offset units, when you see lines appearing in decals, or they seem to flick on and off\nDefault: -2");
	r_drawBuffer = ri.Cvar_Get( "r_drawBuffer", "GL_BACK", CVAR_CHEAT );
	ri.Cvar_SetDescription(r_drawBuffer, "Set which frame buffer to draw into while simultaneously showing the GL_FRONT buffer\nDefault: GL_BACK");
	r_lockpvs = ri.Cvar_Get ("r_lockpvs", "0", CVAR_CHEAT);
	ri.Cvar_SetDescription(r_lockpvs, "Disable update to PVS table as player moves through map (new areas not rendered) for debugging\nDefault: 0");
	r_noportals = ri.Cvar_Get ("r_noportals", "0", CVAR_CHEAT);
	ri.Cvar_SetDescription( r_noportals, "Do not render portals\nDefault: 0");
	r_shadows = ri.Cvar_Get( "cg_shadows", "1", 0 );
	ri.Cvar_SetDescription(r_shadows, "Render player shadows\nDefault: 1");

	r_marksOnTriangleMeshes = ri.Cvar_Get("r_marksOnTriangleMeshes", "0", CVAR_ARCHIVE);
	ri.Cvar_SetDescription(r_marksOnTriangleMeshes, "Show marks on triangle meshes for debugging\nDefault: 0");

	r_aviMotionJpegQuality = ri.Cvar_Get("r_aviMotionJpegQuality", "90", CVAR_ARCHIVE);
	ri.Cvar_SetDescription(r_aviMotionJpegQuality, "Sets the quality for the AVI video recording\nDefault: 90");
	r_screenshotJpegQuality = ri.Cvar_Get("r_screenshotJpegQuality", "90", CVAR_ARCHIVE);
	ri.Cvar_SetDescription(r_screenshotJpegQuality, "Sets the quality for the JPEG video recording\nDefault: 90");

	r_maxpolys = ri.Cvar_Get( "r_maxpolys", va("%d", MAX_POLYS), 0);
	ri.Cvar_SetDescription(r_maxpolys, "Max number of polygons\nDefault: " XSTRING(MAX_POLYS));
	r_maxpolyverts = ri.Cvar_Get( "r_maxpolyverts", va("%d", MAX_POLYVERTS), 0);
	ri.Cvar_SetDescription(r_maxpolyverts, "Max number of polygon vertices to display at a time\nDefault: " XSTRING(MAX_POLYVERTS));

	r_cursorShader = ri.Cvar_Get( "r_inputShader", "cursor", 0);
	r_inputShader = ri.Cvar_Get( "r_inputShader", "cursor", 0);

	// make sure all the commands added here are also
	// removed in R_Shutdown
	ri.Cmd_AddCommand( "imagelist", R_ImageList_f );
	ri.Cmd_SetDescription( "imagelist", "List currently open images/textures used by the map\nUsage: imagelist" );
	ri.Cmd_AddCommand( "shaderlist", R_ShaderList_f );
	ri.Cmd_SetDescription( "shaderlist", "List currently used shaders by the map\nUsage: shaderlist" );
	ri.Cmd_AddCommand( "skinlist", R_SkinList_f );
	ri.Cmd_SetDescription( "skinlist", "List currently used skins by the map\nUsage: shaderlist" );
	ri.Cmd_AddCommand( "modellist", R_Modellist_f );
	ri.Cmd_SetDescription( "modellist", "List currently used models by the map\nUsage: shaderlist" );
	ri.Cmd_AddCommand( "screenshot", R_ScreenShot_f );
	ri.Cmd_SetDescription( "screenshot", "Take a screenshot of the current frame\nUsage: screenshot [silent] [filename]" );
	ri.Cmd_AddCommand( "screenshotJPEG", R_ScreenShotJPEG_f );
	ri.Cmd_SetDescription( "screenshotJPEG", "Take a screenshot of the current frame and output JPEG as the format\nUsage: screenshotJPEG [silent] [filename]" );
	ri.Cmd_AddCommand( "gfxinfo", GfxInfo_f );
	ri.Cmd_SetDescription( "gfxinfo", "Returns extensive information about video settings\nUsage: gfxinfo");
	ri.Cmd_AddCommand( "gfxmeminfo", GfxMemInfo_f );
	ri.Cmd_SetDescription( "gfxmeminfo", "Returns extensive memory information\nUsage: gfxmeminfo");
	ri.Cmd_AddCommand( "exportCubemaps", R_ExportCubemaps_f );
	ri.Cmd_SetDescription( "exportCubemaps", "Export a DDS file of graphics loaded as a cubemap\nUsage: exportCubemaps");
}

void R_InitQueries(void)
{
	if (!glRefConfig.occlusionQuery)
		return;

	if (r_drawSunRays->integer)
		qglGenQueries(ARRAY_LEN(tr.sunFlareQuery), tr.sunFlareQuery);
}

void R_ShutDownQueries(void)
{
	if (!glRefConfig.occlusionQuery)
		return;

	if (r_drawSunRays->integer)
		qglDeleteQueries(ARRAY_LEN(tr.sunFlareQuery), tr.sunFlareQuery);
}


static void RE_SyncRender( void )
{

}


/*
===============
R_Init
===============
*/
void R_Init( void ) {	
	int	err;
	int i;
	byte *ptr;

	ri.Printf( PRINT_ALL, "----- R_Init -----\n" );

	// clear all our internal state
	Com_Memset( &tr, 0, sizeof( tr ) );
	Com_Memset( &backEnd, 0, sizeof( backEnd ) );
	Com_Memset( &tess, 0, sizeof( tess ) );

	if(sizeof(glconfig_t) != 11332)
		ri.Error( ERR_FATAL, "Mod ABI incompatible: sizeof(glconfig_t) == %u != 11332", (unsigned int) sizeof(glconfig_t));

	if ( (intptr_t)tess.xyz & 15 ) {
		ri.Printf( PRINT_WARNING, "tess.xyz not 16 byte aligned\n" );
	}
	//Com_Memset( tess.constantColor255, 255, sizeof( tess.constantColor255 ) );

	//
	// init function tables
	//
	for ( i = 0; i < FUNCTABLE_SIZE; i++ )
	{
		tr.sinTable[i]		= sin( DEG2RAD( i * 360.0f / ( ( float ) ( FUNCTABLE_SIZE - 1 ) ) ) );
		tr.squareTable[i]	= ( i < FUNCTABLE_SIZE/2 ) ? 1.0f : -1.0f;
		tr.sawToothTable[i] = (float)i / FUNCTABLE_SIZE;
		tr.inverseSawToothTable[i] = 1.0f - tr.sawToothTable[i];

		if ( i < FUNCTABLE_SIZE / 2 )
		{
			if ( i < FUNCTABLE_SIZE / 4 )
			{
				tr.triangleTable[i] = ( float ) i / ( FUNCTABLE_SIZE / 4 );
			}
			else
			{
				tr.triangleTable[i] = 1.0f - tr.triangleTable[i-FUNCTABLE_SIZE / 4];
			}
		}
		else
		{
			tr.triangleTable[i] = -tr.triangleTable[i-FUNCTABLE_SIZE/2];
		}
	}

	R_InitFogTable();

	R_NoiseInit();

	R_Register();

	max_polys = r_maxpolys->integer;
	if (max_polys < MAX_POLYS)
		max_polys = MAX_POLYS;

	max_polyverts = r_maxpolyverts->integer;
	if (max_polyverts < MAX_POLYVERTS)
		max_polyverts = MAX_POLYVERTS;

	ptr = ri.Hunk_Alloc( sizeof( *backEndData ) + sizeof(srfPoly_t) * max_polys + sizeof(polyVert_t) * max_polyverts, h_low);
	backEndData = (backEndData_t *) ptr;
	backEndData->polys = (srfPoly_t *) ((char *) ptr + sizeof( *backEndData ));
	backEndData->polyVerts = (polyVert_t *) ((char *) ptr + sizeof( *backEndData ) + sizeof(srfPoly_t) * max_polys);
	R_InitNextFrame();

	InitOpenGL();

	R_InitImages();

	if (glRefConfig.framebufferObject)
		FBO_Init();

	GLSL_InitGPUShaders();

	R_InitVaos();

	R_InitShaders();

	R_InitSkins();

	R_ModelInit();

	R_InitFreeType();

	R_InitQueries();


	err = qglGetError();
	if ( err != GL_NO_ERROR )
		ri.Printf (PRINT_ALL, "glGetError() = 0x%x\n", err);

	// print info
	GfxInfo_f();
	ri.Printf( PRINT_ALL, "----- finished R_Init -----\n" );
}


/*
===============
RE_Shutdown
===============
*/
void RE_Shutdown( refShutdownCode_t code ) {

	ri.Printf( PRINT_ALL, "RE_Shutdown( %i )\n", code );

	ri.Cmd_RemoveCommand( "imagelist" );
	ri.Cmd_RemoveCommand( "shaderlist" );
	ri.Cmd_RemoveCommand( "skinlist" );
	ri.Cmd_RemoveCommand( "modellist" );
	ri.Cmd_RemoveCommand( "modelist" );
	ri.Cmd_RemoveCommand( "screenshot" );
	ri.Cmd_RemoveCommand( "screenshotJPEG" );
	ri.Cmd_RemoveCommand( "gfxinfo" );
	ri.Cmd_RemoveCommand( "gfxmeminfo" );
	ri.Cmd_RemoveCommand( "exportCubemaps" );


	if ( tr.registered ) {
		R_IssuePendingRenderCommands();
		R_ShutDownQueries();
		if (glRefConfig.framebufferObject)
			FBO_Shutdown();
		R_DeleteTextures();
		R_ShutdownVaos();
		Com_Memset(&s_worldData, 0, sizeof(s_worldData));
	}

	R_DoneFreeType();

	// shut down platform specific OpenGL stuff
	if ( code == REF_DESTROY_WINDOW ) {
		if ( tr.registered )
			GLSL_ShutdownGPUShaders();
		ri.GLimp_Shutdown( code == REF_UNLOAD_DLL ? qtrue: qfalse );

		Com_Memset( &glConfig, 0, sizeof( glConfig ) );
		Com_Memset( &glRefConfig, 0, sizeof( glRefConfig ) );

		textureFilterAnisotropic = qfalse;
		maxAnisotropy = 0;

		Com_Memset( &glState, 0, sizeof( glState ) );
	}

	ri.FreeAll();

	tr.registered = qfalse;
}


/*
=============
RE_EndRegistration

Touch all images to make sure they are resident
=============
*/
void RE_EndRegistration( void ) {
	R_IssuePendingRenderCommands();
	if ( !ri.Sys_LowPhysicalMemory() ) {
		RB_ShowImages();
	}
}

void RE_SetDvrFrame(float x, float y, float width, float height) {
	dvrXScale = width;
	dvrYScale = height;
	dvrXOffset = x;
	dvrYOffset = y;
}


/*
@@@@@@@@@@@@@@@@@@@@@
GetRefAPI

@@@@@@@@@@@@@@@@@@@@@
*/
#ifdef USE_RENDERER_DLOPEN
Q_EXPORT refexport_t* QDECL GetRefAPI ( int apiVersion, refimport_t *rimp ) {
#else
refexport_t *GetRefAPI ( int apiVersion, refimport_t *rimp ) {
#endif

	static refexport_t	re;

	ri = *rimp;

	Com_Memset( &re, 0, sizeof( re ) );

	if ( apiVersion != REF_API_VERSION ) {
		ri.Printf(PRINT_ALL, "Mismatched REF_API_VERSION: expected %i, got %i\n", 
			REF_API_VERSION, apiVersion );
		return NULL;
	}

	// the RE_ functions are Renderer Entry points

	re.Shutdown = RE_Shutdown;

	re.BeginRegistration = RE_BeginRegistration;
	re.RegisterModel = RE_RegisterModel;
	re.RegisterSkin = RE_RegisterSkin;
	re.RegisterShader = RE_RegisterShader;
	re.RegisterShaderNoMip = RE_RegisterShaderNoMip;
	re.LoadWorld = RE_LoadWorldMap;
	re.SetWorldVisData = RE_SetWorldVisData;
	re.EndRegistration = RE_EndRegistration;

	re.BeginFrame = RE_BeginFrame;
	re.EndFrame = RE_EndFrame;

	re.MarkFragments = R_MarkFragments;
	re.LerpTag = R_LerpTag;
	re.ModelBounds = R_ModelBounds;

	re.ClearScene = RE_ClearScene;
	re.AddRefEntityToScene = RE_AddRefEntityToScene;
	re.AddPolyToScene = RE_AddPolyToScene;
	re.LightForPoint = R_LightForPoint;
	re.AddLightToScene = RE_AddLightToScene;
	re.AddAdditiveLightToScene = RE_AddAdditiveLightToScene;
	re.RenderScene = RE_RenderScene;

	re.SetColor = RE_SetColor;
	re.DrawStretchPic = RE_StretchPic;
	re.DrawStretchRaw = RE_StretchRaw;
	re.UploadCinematic = RE_UploadCinematic;

	re.RegisterFont = RE_RegisterFont;
	re.RemapShader = R_RemapShader;
	re.GetEntityToken = R_GetEntityToken;
	re.inPVS = R_inPVS;

	re.TakeVideoFrame = RE_TakeVideoFrame;
	re.SetColorMappings = R_SetColorMappings;

	re.ThrottleBackend = RE_ThrottleBackend;
	re.CanMinimize = RE_CanMinimize;
	re.GetConfig = RE_GetConfig;
	re.VertexLighting = RE_VertexLighting;
	re.SyncRender = RE_SyncRender;

	re.SetDvrFrame = RE_SetDvrFrame;
	re.CreateShaderFromImageBytes = RE_CreateShaderFromImageBytes;
#ifdef USE_VID_FAST
	re.UpdateMode = RE_UpdateMode;
#endif
	re.FastCapture = RB_FastCapture;
	re.FastCaptureOld = RB_FastCaptureOld;
#ifdef USE_LAZY_MEMORY
	re.ReloadShaders = RE_ReloadShaders;
	re.SwitchWorld = RE_SwitchWorld;
#endif
#ifdef USE_LAZY_LOAD
	re.UpdateShader = RE_UpdateShader;
	re.UpdateModel = R_UpdateModel;
#endif
	re.ResetBannerSpy = RE_ResetBannerSpy;

	return &re;
}


#ifdef USE_VID_FAST
/*
=============
RE_UpdateMode
=============
*/
void RE_UpdateMode(glconfig_t *glconfigOut) {
	R_IssuePendingRenderCommands();

	glconfigOut = &glConfig;
	ri.GLimp_UpdateMode( glconfigOut );

	GL_SetDefaultState();

	GL_CheckErrors();

	*glconfigOut = glConfig;
}
#endif
