/*=======================================================================
 *** THE CONTENT OF THIS WORK IS PROPRIETARY TO FEI S.A.S, (FEI S.A.S.),            ***
 ***              AND IS DISTRIBUTED UNDER A LICENSE AGREEMENT.                     ***
 ***                                                                                ***
 ***  REPRODUCTION, DISCLOSURE,  OR USE,  IN WHOLE OR IN PART,  OTHER THAN AS       ***
 ***  SPECIFIED  IN THE LICENSE ARE  NOT TO BE  UNDERTAKEN  EXCEPT WITH PRIOR       ***
 ***  WRITTEN AUTHORIZATION OF FEI S.A.S.                                           ***
 ***                                                                                ***
 ***                        RESTRICTED RIGHTS LEGEND                                ***
 ***  USE, DUPLICATION, OR DISCLOSURE BY THE GOVERNMENT OF THE CONTENT OF THIS      ***
 ***  WORK OR RELATED DOCUMENTATION IS SUBJECT TO RESTRICTIONS AS SET FORTH IN      ***
 ***  SUBPARAGRAPH (C)(1) OF THE COMMERCIAL COMPUTER SOFTWARE RESTRICTED RIGHT      ***
 ***  CLAUSE  AT FAR 52.227-19  OR SUBPARAGRAPH  (C)(1)(II)  OF  THE RIGHTS IN      ***
 ***  TECHNICAL DATA AND COMPUTER SOFTWARE CLAUSE AT DFARS 52.227-7013.             ***
 ***                                                                                ***
 ***                   COPYRIGHT (C) 1996-2014 BY FEI S.A.S,                        ***
 ***                        MERIGNAC, FRANCE                                        ***
 ***                      ALL RIGHTS RESERVED                                       ***
**=======================================================================*/
/*=======================================================================
** Author      : Pascal Estrade (Apr 2000)
** Modified by : Mike Heck (Aug 2000)
** Modified by : Thierry Dufour (Sep 2000)
** Modified by : Pascal Estrade (Dec 2000)
** Modified by : Jerome Hummel (Dec 2002)
**=======================================================================*/



#include "volRend.h"
#include <Inventor/helpers/SbFileHelper.h>

#include <Inventor/antialiasing/SoAntialiasingParameters.h>

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
//
// Make the color map completely transparent *except*
// in the specified range (where alpha is left alone)
//
// Modified from Paul's original code.  Original took
// a list of float values, this one takes an array of
// unsigned ints, each of which is a packed RGBA value.
//
void updateOpaqueRegion( SoTransferFunction *tf, int minCm, int maxCm, int minOp, int maxOp, SbBool invertFlag )
{
  {
    // If min is zero, no change.
    // If min is less than zero, also no change (bogus case).
    tf->minValue = minCm;
    tf->maxValue = maxCm;

    int length = tf->actualColorMap.getNum() / 4;
    float *actualColorMapF = tf->actualColorMap.startEditing();
    if (actualColorMapF != NULL)
    {
      if(!invertFlag)//remap inside min map
      {
        // If min is zero, no change.
        // If min is less than zero, also no change (bogus case).
        for (int i = 0; i < minOp; i++)
          actualColorMapF[4*i+3] = 0.;
        // If max = length-1 (last index in color map), no change.
        // If max > length-1, also no change (bogus case).
        for (int k = maxOp+1; k < length; k++)
          actualColorMapF[4*k+3] = 0.;
      }
      else
      {
        for (int i = minOp; i <= maxOp; i++)
          actualColorMapF[4*i+3] = 0.;
      }
    }
    tf->actualColorMap.finishEditing();
  }
}

////////////////////////////////////////////////////////////////////////
//
SbString getDataFilePath( const SbString &filename )
{
  // Ignore bogus input (a NULL can crash strcpy)
  if (! filename)
    return SbString((char*)NULL);

  return filename;
}

SoUniformGridClipping *createDefaultGrid(SbBool createDistanceTexture)
{
  SoUniformGridClipping *grid = new SoUniformGridClipping;
  SbString fname = getDataFilePath("clipmaps/perlin1.png");
  if( fname.isEmpty() || createDistanceTexture)
    genDefaultClipMap(grid);
  else
    grid->filename = fname;
  return grid;
}

void genDefaultClipMap(SoUniformGridClipping *grid)
{
  SbVec2s size(256, 256);
  unsigned char *data = new unsigned char[size[0]*size[1]];
  int offs = 0;

  double dmax = sqrt((double)(size[0]*size[1]/4+size[0]*size[1]/4));
  for(int j = 0; j < size[1]; j++)
    for(int i = 0; i < size[0]; i++) {
      double d = sqrt((double)((i-size[0]/2)*(i-size[0]/2)+(j-size[1]/2)*(j-size[1]/2)))*255/dmax;
      data[offs++] = static_cast<unsigned char>(255.0f-(unsigned char)d);
    }

  grid->image.setValue(size, 1, SoSFImage::UNSIGNED_BYTE, data, SoSFImage::NO_COPY_AND_DELETE);
  grid->internalFormat = SoUniformGridClipping::LUMINANCE8;
}

////////////////////////////////////////////////////////////////////////
//
// Read a color map from a CLUT1999a file (TeraRecon format)
//
// This file should begin with header like this:
//
//   Clut1999a
//   256
//   RGBRange 255
//   AlphaRange 4095
//   Title Revli Generated Lookup Table
//   CopyRight none
//   ##
//
// Followed by one or more blank lines.
// Followed by N lines of 5 int values (index,r,g,b,a)

int loadClut1999( FILE *file, int &numComps, float *&values )
{
  const int BUFSIZE = 1024;
  char buf[BUFSIZE];

  // Get first line of file and confirm identity
  fgets( buf, BUFSIZE, file );
  if (strncmp( buf, "Clut1999", 8)) {
    return 0;
  }

  // Get the rest of the header
  int result;
  int numEntries, rgbRange, alphaRange;
  result = fscanf( file, "%d", &numEntries );
  if (result < 1 || numEntries == 0) return 0; // Error

  result = fscanf( file, "%s %d", buf, &rgbRange );
  if (result < 1) return 0; // Error

  result = fscanf( file, "%s %d", buf, &alphaRange );
  if (result < 1) return 0; // Error

  // Read to end of header
  while (1) {
    char *p = fgets( buf, BUFSIZE, file );
    if (p == NULL)
      return 0; // Error
    if (! strncmp(buf, "##", 2))
      break;
  }

  // Read values
  int numFloats = numEntries * 4;
  float *rgba = new float[numFloats];
  for (int i = 0; i < numFloats; ++i)
    rgba[i] = 0;

  float rgbScale = 1.f / (float)rgbRange;
  float alphaScale = 1.f / (float)alphaRange;

  int iia = 0;

  while (1) {
    int index, ir, ig, ib, ia;
    result = fscanf( file, "%d %d %d %d %d", &index, &ir, &ig, &ib, &ia );
    if (result == EOF)
      break;
    if (result < 5)
      continue; // Blank line ???

    float *p = rgba + (index * 4);
    *p++ = ir * rgbScale;
    *p++ = ig * rgbScale;
    *p++ = ib * rgbScale;
    *p   = ia * alphaScale;
    iia=(int)(*p*4095);
  }

  numComps = 4;
  values   = rgba;
  return numEntries;
}

////////////////////////////////////////////////////////////////////////
//
// Read a color map from an Amira .col file (AmiraMesh Lattice format)
//
// File should begin with header like this:
//
//      # AmiraMesh 3D ASCII 1.0
//      #
//      # CreationDate: Sat Apr 25 09:52:28 1998
//
//      define Lattice 256
//
//      Parameters {
//          ContentType "Colormap",
//          Filename "/onyx1/people/malte/Amira/data/colormaps/volrenRed.icol",
//          MinMax 10 180 }
//
//      Lattice { float[4] Data } @1
//
//      # Data section follows
//      @1
//      1 0 0 0
//      ...
//

int loadAmiraColormap( FILE *file, int &numComps, float *&values )
{
  const int BUFSIZE = 1024;
  char buf[BUFSIZE];
  char *p = buf;

  // Get first line of file and confirm identity
  fgets( buf, BUFSIZE, file );
  while (*p == ' ') p++; // skip whitespace
  if (*p == '#') p++;    // skip comment char
  while (*p == ' ') p++; // skip whitespace
  if (*p == '\0' || strncmp( p, "AmiraMesh", 9)) {
    return 0;
  }

  // Get the rest of the header
  int result, numEntries;

  // Find "define Lattice"
  numEntries = 0;
  while (1) {
    p = fgets( buf, BUFSIZE, file );
    if (p == NULL) // Error
      return 0;
    while (*p == ' ') p++;
    if (!strncmp( p, "define", 6)) {
      while (*p && *p != ' ') p++;
      while (*p == ' ') p++;
      if (*p && !strncmp(p,"Lattice",7)) {
        while (*p && *p != ' ') p++;
        while (*p == ' ') p++;
        if (*p) {
          numEntries = atoi( p );
          break;
        }
      }
    }
  }
  if (numEntries == 0) // Error
    return 0;

  // Find 'contentType "Colormap"'
  int isColormapFile = 0;
  while (1) {
    p = fgets( buf, BUFSIZE, file );
    if (p == NULL) // Error
      return 0;
    while (*p == ' ') p++;
    if (!strncmp( p, "ContentType", 11)) {
      while (*p && *p != ' ') p++;
      while (*p == ' ') p++;
      if (*p && !strncmp(p,"\"Colormap\"",10)) {
        isColormapFile = 1;
        break;
      }
    }
  }
  if (isColormapFile == 0) // Error
    return 0;

  // Find "@1" at beginning of line
  int foundDataSection = 0;
  while (1) {
    p = fgets( buf, BUFSIZE, file );
    if (p == NULL) // Error
      return 0;
    while (*p == ' ') p++;
    if (!strncmp( p, "@1", 2)) {
      foundDataSection = 1;
      break;
    }
  }
  if (foundDataSection == 0) // Error
    return 0;

  // Read values
  int numFloats = numEntries * 4;
  float *rgba = new float[numFloats];
  float *pflt = rgba;
  for (int i = 0; i < numFloats; ++i)
    *pflt++ = 0;

  int count = 0;
  pflt = rgba;
  while (1) {
    float r, g, b, a;
    result = fscanf( file, "%g %g %g %g", &r, &g, &b, &a );
    if (result == EOF)
      break;
    if (result < 4)
      continue; // Blank line ???

    *pflt++ = r;
    *pflt++ = g;
    *pflt++ = b;
    *pflt++ = a;
    count++;
    if (count == numEntries)
      break;
  }

  numComps = 4;
  values   = rgba;
  return numEntries;
}


////////////////////////////////////////////////////////////////////////
//
// Read a color map from a file
//
// First checks for known 3rd party formats, e.g. TeraRecon CLUT1999a
//
// Default is simple VolumeViz example file format.
// This format only supports RGB values (no alpha)
// and the values range from 0..65535 (16 bits).
//
// First line contains 1 int: number of values
// Then each line contains 3 ints: R, G and B
//
// If there are less than 256 values, values are
// replicated to make 256 values.  Therefore number
// of values should always be a power of 2.
//
// old: float *getColorTable( const char *colorFile )
//
// input:
//   colorFile: pathname of file
// output:
//   numComps: number of components in each value (1, 2 or 4)
//   values  : array of numEntries*numComps float values
//             This memory is allocated in the function!
// returns:
//   numEntries: number of entries in colormap (usually 256)
//               or zero if error loading colormap

int loadColorTable( const SbString& colorFile, int &numComps, float *&values )
{
  FILE *file;
  const int BUFSIZE = 1024;
  char buf[BUFSIZE];

  numComps = 0;
  values   = NULL;

  SbString fullName;
  SbBool found = SoInput::findAbsolutePath(colorFile, fullName );

  if ( !found || (file = SbFileHelper::open(fullName, "r") ) == NULL)
  {
    char buf[1024];
    sprintf( buf, "Can't open the color map file '%s'\n", colorFile.toLatin1() );
    DISPLAY_ERROR( buf );
    return 0;
  }

  // Get first line of file and try to identify it.
  char *p = fgets( buf, BUFSIZE, file );
  if (!p)
    return 0;
  int num_colors = 0;

  // If Amira format, use specific file loader
  while (*p == ' ') p++; // skip whitespace
  if (*p == '#') p++;    // skip comment char
  while (*p == ' ') p++; // skip whitespace
  if (*p && !strncmp( p, "AmiraMesh", 9))
  {
    fseek( file, 0, SEEK_SET );
    num_colors = loadAmiraColormap( file, numComps, values );
    fclose( file );
    return num_colors;
  }

  // If TeraRecon format, use specific file loader
  if (! strncmp( buf, "Clut1999", 8))
  {
    // Rewind file to avoid confusion
    fseek( file, 0, SEEK_SET );
    num_colors = loadClut1999( file, numComps, values );
    fclose( file );
    return num_colors;
  }

  // Process simple format
//  fscanf(file, "%d", &num_colors);
  sscanf(buf, "%d", &num_colors);
  if (num_colors <= 0)
    return 0;

  float rgba[256][4];

  int numComponents = 0;
  for (int i = 0; i < num_colors; )
  {
    char line[256];
    fgets( line, 256, file );
    float color[4];
    numComponents = sscanf( line, "%g %g %g %g", &color[0], &color[1], &color[2], &color[3]);
    for (int k = 0; k < numComponents; k++)
      color[k] /= 65535.0f;
    switch (numComponents)
    {
        case 1: // A (RGB=1)
          color[3] = color[0];
          color[0] = color[1] = color[2] = 1;
          break;
        case 3: // RGB (A=1)
          color[3] = 1;
          break;
        case 4: // RGBA
          break;
        default: // empty line
          continue;
    }
    rgba[i][0] = color[0];
    rgba[i][1] = color[1];
    rgba[i][2] = color[2];
    rgba[i][3] = color[3];
    i++;
  }
  fclose(file);

  int mode = 256 / num_colors;

  static float rgba256[256][4];
  int index = 0;
  for (int k = 0; k < num_colors; k++)
  {
    int k1 = k + 1;
    if (k1 >= num_colors) k1 = num_colors-1;
    for (int i = 0; i < mode; i++)
    {
      rgba256[index][0] = rgba[k][0] + (float)i/mode*(rgba[k1][0]-rgba[k][0]);
      rgba256[index][1] = rgba[k][1] + (float)i/mode*(rgba[k1][1]-rgba[k][1]);
      rgba256[index][2] = rgba[k][2] + (float)i/mode*(rgba[k1][2]-rgba[k][2]);
      rgba256[index][3] = rgba[k][3] + (float)i/mode*(rgba[k1][3]-rgba[k][3]);
      index++;
    }
  }

  //  return &rgba256[0][0];
  numComps = 4;
  values   = &rgba256[0][0];
  return 256;
}

unsigned char *testData = NULL;
void initDataNode(SoVolumeData * volData, const SbString &filepath)
{
  if(testData){
    delete[] testData;
    testData = NULL;
  }

  // Set volume rendering parameters
  // Note: This should be done first in case the file path is NULL
  //       and we just create fake data (and return from this function).
  //
  // TODO: Make a new getInt that takes an allowed range of values.
  //       Clamp the incoming value or replace invalid with default.
  int value;
  value = SoPreferences::getInt( "VOLREND_mainMemSize", -1 ); // 1 - 512
  if (value >= 0) 
    volData->ldmResourceParameters.getValue()->maxMainMemory = value;

  value = SoPreferences::getInt( "VOLREND_texMemSize", -1 ); // 1 - 128
  if (value >= 0)
    SoLDMGlobalResourceParameters::setMaxTexMemory( value );

  value = SoPreferences::getInt( "VOLREND_texLoadRate", -1 ); // 0 - 64
  if (value >= 0)
    SoLDMGlobalResourceParameters::setTex3LoadRate( value );

  value = SoPreferences::getInt( "VOLREND_sliceNumTex", -1 ); // 1 - 1024
  if (value >= 0)
    volData->ldmResourceParameters.getValue()->max2DTextures = value;

  value = SoPreferences::getInt( "VOLREND_sliceTexLoadRate", -1 ); // 1- 256
  if (value >= 0)
    volData->ldmResourceParameters.getValue()->tex2LoadRate = value;

    if (filepath.isEmpty() )
  {
    // If default file was not found, make some trivial test data.
    // More convenient for debugging if these values are *not* const
    // (can be changed during execution). The memory allocated is a
    // huge leak of course, but it's just for a demo and testing. :-)
#if 0
    int testXdim = 5;
    int testYdim = 5;
    int testZdim = 8;
    int numTestBytes = testXdim * testYdim * testZdim;
    static unsigned char *testData = new unsigned char[numTestBytes];
    memset( testData, 254, numTestBytes );
    volData->setVolumeSize( SbBox3f(-testXdim,-testYdim,-testZdim,testXdim,testYdim,testZdim) );
    volData->setVolumeData( SbVec3i32(testXdim,testYdim,testZdim), testData );
#else

  char buf[256];
  sprintf( buf, "VolRend: Could not locate specified data file - Creating default memory data");
#if defined(_WIN32) && !defined(SOQT)
  SbString str(buf) ;
  SoConsole* console = SoXt::getErrorConsole();
  console->printMessage(str);
#else
    printf("%s\n", buf) ;
#endif
    int testXdim = 50;
    int testYdim = 50;
    int testZdim = 50;
    int numTestBytes = testXdim * testYdim * testZdim;
    testData = new unsigned char[numTestBytes];
    /*for (int i = 0; i < numTestBytes; i++)
    testData[i] = i%256;*/

    //create a somehow interesting default data.
    int counter = 0;
    for (int i=0; i<testXdim; i++){
      for (int j=0; j<testYdim; j++){
        for (int k=0; k < testZdim; k++){
          double phase = sin(2*PI*i/50.) + cos(2*PI*k/50.) + j/4. ;
          testData[counter] = char(127 + 127*sin( phase ));
          counter ++;
        }
      }
    }

    volData->extent = SbBox3f( (float)(-testXdim), (float)(-testYdim), (float)(-testZdim), (float)testXdim, (float)testYdim, (float)testZdim );
    volData->data.setValue(SbVec3i32(testXdim,testYdim,testZdim), SbDataType::UNSIGNED_BYTE, 0, testData, SoSFArray::NO_COPY);
#endif
    return;
  }

  SbString name = SbFileHelper::getBaseName(filepath);

  const char *useLogo = SoPreferences::getValue("VOLREND_useLogo");
  if(useLogo){
    fprintf(stderr, "Logo to use: %s\n", useLogo);
    SbString dataLogo = SoPreferences::getString("VOLREND_dataLogo","");
    fprintf(stderr, "Associated data: %s\n", dataLogo.toLatin1());
    if(dataLogo.isEmpty())
    {
      SoError::post("To use logo, please specify data name to use logo with using VOLREND_dataLogo env var");
    }
    else{
      fprintf(stderr, "filename: %s\n", name.toLatin1());
      if( name == dataLogo)
      {
        m_courtesyLogo->filename = useLogo;
        m_logoSwitch->whichChild = SO_SWITCH_ALL;
        m_imgSwitch->whichChild = SO_SWITCH_NONE;
      }
      else
      {
        m_logoSwitch->whichChild = SO_SWITCH_NONE;
        m_imgSwitch->whichChild = SO_SWITCH_ALL;
      }
    }
  }

  SbString fileExt = SbFileHelper::getExtension(name);
  if ( !fileExt.isEmpty() )
  {
    volData->resetReader();
    if ( fileExt.upper() == "RAW" )
    {
      SoVRGenericFileReader *rawReader = new SoVRGenericFileReader();
      SoVolumeData::DataType tType = SoVolumeData::UNSIGNED_BYTE;
      rawReader->setFilename(filepath);
      if ( name.upper() == "EXAMPLE.RAW" )
      {
        SbBox3f   tmpF(-1.0,-1.0,-1.0,1.0,1.0,1.0);
        SbVec3i32 tmpS(50,50,50);
        rawReader->setDataChar(tmpF, tType, tmpS);
      }
      volData->setReader(*(rawReader), TRUE);
    }
    else
      volData->fileName.setValue( filepath );

    //for sgy file, rotate the camera so that the scene come up oriented correctly
    bool rotateScene = false;
    if( fileExt.upper().contains("LDM") || fileExt.upper().contains("LDA"))
    {
      if ( m_volData->getReader() )
      {
        SbString origin = m_volData->getReader()->getOriginalFilename();
        if(!origin.isEmpty())
        {
          SbString fileExt = SbFileHelper::getExtension(origin);
          if(fileExt.upper().contains("EGY") || fileExt.upper().contains("SGY"))
          {
            rotateScene = true;
          }
        }
      }
    }
    if( fileExt.upper().contains("EGY") || fileExt.upper().contains("SGY"))
    {
      //rotateScene = true;
      //volData->getReader()->setDirectCoorSys(FALSE);
    }

    if(rotateScene)
    {
      m_segySwitch->whichChild = SO_SWITCH_ALL;
    }
    else
      m_segySwitch->whichChild = SO_SWITCH_NONE;
  }
}


#ifdef SPACEBALL
#include "VolumeVizAuditor.h"
extern VolumeVizAuditor *myAuditor;

//SpaceBall
void spaceballButtonCB(void *userData, SoEventCallback *cb)
{
  const SoSpaceballButtonEvent *ev =
    (const SoSpaceballButtonEvent *) cb->getEvent();
  sb_stuct *localSbStruc = (sb_stuct *) userData;

  if (ev->getState() == SoButtonEvent::DOWN) {
    int which = ev->getButton();
    switch (which) {
    case 1:
      m_rotationMode    = 1;
      m_translationMode = 0;
      m_colorMapMode = 0;
      break;
    case 2:
      m_translationMode = 1;
      m_rotationMode    = 0;
      m_colorMapMode = 0;
      break;
    case 3:
      m_translationMode = 1;
      m_rotationMode    = 1;
      m_colorMapMode = 0;
      break;
    case 4:
      m_colorMapMode = 1;
      m_translationMode = 0;
      m_rotationMode    = 0;
      break;
    case 5:
      localSbStruc->sb_struct_trans->translation.setValue(SbVec3f(0.0,0.0,0.0));
      localSbStruc->sb_struct_rot->rotation.setValue(SbRotation(0.0,0.0,0.0,1.0));
      m_viewer->resetToHomePosition();
      break;
    default:
      break;
    }
  }
}

void
motion3TranslationCB(void *userData, SoEventCallback *cb)
{
  if (!m_translationMode && !m_colorMapMode)
    return;

  const SoMotion3Event *ev = (const SoMotion3Event *) cb->getEvent();
  sb_stuct *localSbStruc = (sb_stuct *) userData;

  // If we are in translation Mode
  if (m_translationMode) {
    localSbStruc->sb_struct_trans->translation =
      localSbStruc->sb_struct_trans->translation.getValue() + ev->getTranslation();
  }

  // If we are in colorMap Mode
  else if (m_colorMapMode) {

    int stepX, stepY, stepZ;
    stepX = stepY = stepZ = 0;

    // Update the min ColorMap
    if ( (fabs(ev->getTranslation()[0]) > fabs(ev->getTranslation()[1])) &&
      (fabs(ev->getTranslation()[0]) > fabs(ev->getTranslation()[2])) ) {

        stepX = (int)(ev->getTranslation()[0]/fabs(ev->getTranslation()[0]));
        m_minColorMap += stepX;
        if (m_minColorMap < 0 )
          m_minColorMap = 0;
        if (localSbStruc->sb_next_colorMap)
          localSbStruc->sb_next_colorMap = FALSE;

      }

      // Update the max ColorMap
    else if ( (fabs(ev->getTranslation()[2]) > fabs(ev->getTranslation()[0])) &&
      (fabs(ev->getTranslation()[2]) > fabs(ev->getTranslation()[1])) ) {

        stepY = (int)(ev->getTranslation()[2]/fabs(ev->getTranslation()[2]));
        m_maxColorMap -= stepY;
        if (m_maxColorMap > 255 )
          m_maxColorMap = 255;
        if (localSbStruc->sb_next_colorMap)
          localSbStruc->sb_next_colorMap = FALSE;

      }

      // Change the ColorMap
    else if ( (fabs(ev->getTranslation()[1]) > fabs(ev->getTranslation()[0])) &&
      (fabs(ev->getTranslation()[1]) > fabs(ev->getTranslation()[2])) ) {

        stepZ = (int)(ev->getTranslation()[1]/fabs(ev->getTranslation()[1]));

        // If the user push or pull the spaceBall in the vertical direction
        if ( !(localSbStruc->sb_next_colorMap) && (fabs(ev->getTranslation()[2]) > 15 * SB_STEP) ) {

          SoDialogComboBox *combo =
            (SoDialogComboBox *)myTopLevelDialog->searchForAuditorId("colormap");

          if ( (combo->selectedItem.getValue() == 0) && (stepZ > 0) )
            combo->selectedItem.setValue( combo->items.getNum() -1);
          else
            combo->selectedItem.setValue( ( combo->selectedItem.getValue() - stepZ )%( combo->items.getNum()) );

          //Update dialog
          myAuditor->doCmapChange(combo->selectedItem.getValue(),m_transferFunction);
          localSbStruc->sb_next_colorMap = TRUE;
        }

        // If the user release spaceBall
        if ((localSbStruc->sb_next_colorMap) && (fabs(ev->getTranslation()[2]) < 2.1 * SB_STEP))
          localSbStruc->sb_next_colorMap = FALSE;

        return;
      }

    else {
      return;
    }

    // If min and max ColorMap are equal, block the previous command
    if ( m_minColorMap == m_maxColorMap) {
      m_minColorMap -= stepX;
      m_maxColorMap -= stepY;
    }

    m_transferFunction->minValue = m_minColorMap;
    m_transferFunction->maxValue = m_maxColorMap;

    //update the dialog
    SoDialogIntegerSlider* colorMapMax = (SoDialogIntegerSlider*)myTopLevelDialog->searchForAuditorId(SbString("colormap max"));
    colorMapMax->value.setValue(m_maxColorMap);
    SoDialogIntegerSlider* colorMapMin = (SoDialogIntegerSlider*)myTopLevelDialog->searchForAuditorId(SbString("colormap min"));
    colorMapMin->value.setValue(m_minColorMap);
  }
}

void
motion3RotationCB(void *userData, SoEventCallback *cb)
{
  if (!m_rotationMode) return;

  const SoMotion3Event *ev = (const SoMotion3Event *) cb->getEvent();
  sb_stuct *localSbStruc = (sb_stuct *) userData;
  localSbStruc->sb_struct_rot->rotation =
    localSbStruc->sb_struct_rot->rotation.getValue() * ev->getRotation();
}
#endif//spaceball

void
motionSliceCallback(void * /*user_data*/, SoJackDragger *dragger)
{
  m_draggerSlicePos = dragger->translation.getValue();
  float x, y, z;
  m_draggerSlicePos.getValue(x,y,z);
  int sliceNumber;
  switch(m_sliceOri) {
  case X:
    sliceNumber = (int)(m_width / 2.0f + ((int)m_width / 2 / m_maxWidth * x));
    break;
  case Y:
    sliceNumber = (int)(m_height / 2.0f + ((int)m_height / 2 / m_maxHeight * y));
    break;
  case Z:
    sliceNumber = (int)(m_depth / 2.0f + ((int)m_depth / 2 / m_maxDepth * z));
    break;
  }
  m_orthoTab[m_currentSlice]->sliceNumber.setValue( sliceNumber );
#ifndef NO_DIALOG
  //  m_OrthoSliceSlider->setValue( sliceNumber );
#endif
}

////////////////////////////////////////////////////////////////////////
//
void
motionObliSliceCallback(void * /*user_data*/, SoJackDragger *dragger) {
  SbVec3f planeNormal(0,0,1) ;
  SbVec3f draggerPos = dragger->translation.getValue();
  // get the dragger scale factor
  SbVec3f draggerScale = dragger->scaleFactor.getValue();

  // rotate the plane's normal by the dragger rotation
  SbRotation rotation = dragger->rotation.getValue();
  rotation.multVec(SbVec3f(0,1,0),planeNormal);

  // translate cross section and cross contour
  m_obliSlice->plane.setValue(SbPlane(planeNormal,draggerPos));
}

////////////////////////////////////////////////////////////////////////
//
SoSeparator*
createColorMapLegend( SoTransferFunction *pXferNode )
{
  // Legend (color map)
  const float legXmin = -.99f;
  const float legXmax = -.89f;
  const float legYmin = -.8f;
  const float legYmax =  .8f;
  const float legFaceVerts1[][3] =
  {{legXmin,legYmin,0}, {legXmax,legYmin,0},
  {legXmax,legYmax,0}, {legXmin,legYmax,0}};
  const float legFaceVerts2[][3] =
  {{legXmin,legYmin,0.0002f}, {legXmax,legYmin,0.0002f},
  {legXmax,legYmax,0.0002f}, {legXmin,legYmax,0.0002f}};
  const float legLineVerts[][3] =
  {{legXmin,legYmin,0.0004f}, {legXmax,legYmin,0.0004f},
  {legXmax,legYmax,0.0004f}, {legXmin,legYmax,0.0004f}};
  const int32_t legLineIndex[] = {0, 1, 2, 3, 0};

  unsigned char pChecksImage[4] = {0x00, 0xff, 0xff, 0x00};
  int32_t texIndex[] = {0,1,2,3,0};
  float texCoord1[][2] = {{0,1.5},{0,0},{10,0},{10,1.5}};
  float texCoord2[][2] = {{0,1},{0,0},{1,0},{1,1}};

  // LineSet to outline colormap
  SoVertexProperty *pLegProp = new SoVertexProperty;
  pLegProp->vertex.setValues( 0, 4, legLineVerts );
  pLegProp->orderedRGBA.set1Value( 0, 0xFFFFFFFF );
  SoIndexedLineSet *pLegLine = new SoIndexedLineSet;
  pLegLine->vertexProperty = pLegProp;
  pLegLine->coordIndex.setValues( 0, 5, legLineIndex );

  // First FaceSet = checkerboard
  SoTexture2       *pLegTex1  = new SoTexture2;
  SoVertexProperty *pLegProp1 = new SoVertexProperty;
  SoIndexedFaceSet *pLegFace1 = new SoIndexedFaceSet;

  float legLength = legYmax - legYmin;
  float legHeight = legXmax - legXmin;
  float legAspect = 1.5f * legLength / legHeight;
  texCoord1[2][0] = legAspect;
  texCoord1[3][0] = legAspect;
  pLegTex1->image.setValue( SbVec2s(2,2), 1, pChecksImage );
  pLegTex1->magFilter = SoTexture::NEAREST;
  pLegTex1->minFilter = SoTexture::NEAREST;

  pLegProp1->vertex.setValues( 0, 4, legFaceVerts1 );
  pLegProp1->orderedRGBA.set1Value( 0, 0xFFFFFFFF );
  pLegProp1->texCoord.setValues( 0, 4, texCoord1 );
  pLegFace1->vertexProperty = pLegProp1;
  pLegFace1->coordIndex.setValues( 0, 5, legLineIndex );
  pLegFace1->textureCoordIndex.setValues( 0, 5, texIndex );

  // Second FaceSet = colormap
  SoTexture2       *pLegTex2  = new SoTexture2;
  pLegTex2->magFilter = SoTexture::NEAREST;
  pLegTex2->minFilter = SoTexture::NEAREST;
  SoVertexProperty *pLegProp2 = new SoVertexProperty;
  SoIndexedFaceSet *pLegFace2 = new SoIndexedFaceSet;

  updateColorMapLegend( pXferNode, pLegTex2 );
  pLegProp2->vertex.setValues( 0, 4, legFaceVerts2 );
  pLegProp2->orderedRGBA.set1Value( 0, 0xFFFFFFFF );
  pLegProp2->texCoord.setValues( 0, 4, texCoord2 );
  pLegFace2->vertexProperty = pLegProp2;
  pLegFace2->coordIndex.setValues( 0, 5, legLineIndex );
  pLegFace2->textureCoordIndex.setValues( 0, 5, texIndex );

  // Pull the legend out close to the near clip plane
  // (Note this depends on how the app setup the camera...)
  if(!m_colorMapTrans){
    m_colorMapTrans = new SoTranslation;
    m_colorMapTrans->translation.setValue( SbVec3f(0,0,.499f) );
  }

  SoPolygonOffset *pFaceOffset = new SoPolygonOffset;
  pFaceOffset->styles = SoPolygonOffset::FILLED;
  pFaceOffset->units  = -5;
  SoPolygonOffset *pLineOffset = new SoPolygonOffset;
  pLineOffset->styles = SoPolygonOffset::LINES;
  pLineOffset->units  = -10;

  // Build the legend scene graph
  SoSeparator *pLegSep = new SoSeparator;
  pLegSep->addChild( m_colorMapTrans );
  pLegSep->addChild( pLegTex1 );
  pLegSep->addChild( pLegFace1 );
  pLegSep->addChild( pFaceOffset );
  pLegSep->addChild( pLegTex2 );
  pLegSep->addChild( pLegFace2 );
  pLegSep->addChild( new SoTexture2 );    // Turn off texturing before drawing lines
  pLegSep->addChild( pLineOffset );
  pLegSep->addChild( pLegLine );

  // Setup to monitor this transfer function
  SoNodeSensor *pXferSensor = new SoNodeSensor( colorMapSensorCB, (void*)pLegTex2 );
  pXferSensor->attach( pXferNode );
  SoNodeSensor *pSliceXferSensor = new SoNodeSensor( colorMapSensorCB, (void*)pLegTex2 );
  pSliceXferSensor->attach( m_sliceTransferFunction );

  return pLegSep;
}

SoSeparator*
createLoadingSign()
{
  // Pull the legend out close to the near clip plane
  // (Note this depends on how the app setup the camera...)
  if(!m_loadingSignTrans){
    m_loadingSignTrans = new SoTranslation;
    m_loadingSignTrans->translation.setValue( SbVec3f( 0.9f ,0.9f, 0.5f) );
  }

  SoSphere* loadingSign = new SoSphere;
  loadingSign->radius = .07f;

  if ( !m_loadingSignMat )
    m_loadingSignMat = new SoMaterial;
  m_loadingSignMat->diffuseColor = SbColor( 0., 1., 0.);

  SoSeparator* sep = new SoSeparator;
  sep->addChild( m_loadingSignTrans );
  sep->addChild( m_loadingSignMat );
  sep->addChild( loadingSign);
  return sep;
}

void loadCB(SbBool startLoading, void* /*userData*/ )
{
  m_loadCBMutex->lock();
  {
    if( startLoading )
      m_loadSignColor.setValue( 1., 0., 0.);
    else
      m_loadSignColor.setValue( 0., 1., 0.);
    m_changeMaterial = TRUE;
  }
  m_loadCBMutex->unlock();
}

void redrawLoadingSign(void * /*data*/, SoSensor * /*sensor*/)
{
  m_loadCBMutex->lock();
  {
    if( m_changeMaterial ){
      SbBool saveEnableNotify = m_loadingSignMat->enableNotify(FALSE);
      m_loadingSignMat->diffuseColor.setValue( m_loadSignColor );
      m_changeMaterial = FALSE;
      m_loadingSignMat->enableNotify(saveEnableNotify);
    }
  }
  m_loadCBMutex->unlock();
}

///////////////////////////////////////////////////////////////////////
//
void updateVolBBoxGeometry()
{
  if (m_volBBoxSwitch) {
    SoSeparator *pBboxRoot =
      (SoSeparator*)SoNode::getByName("VOL_BBOX_GEOM");
    if (!pBboxRoot) {
      pBboxRoot = new SoSeparator;
      pBboxRoot->setName( "VOL_BBOX_GEOM" );
      m_volBBoxSwitch->addChild( pBboxRoot );
    }
    else {
      pBboxRoot->removeAllChildren();
    }

    // Get volume geometry (bounds in modeling coords)
    SbBox3f volSize = m_volData->extent.getValue();
    float xmin,ymin,zmin,xmax,ymax,zmax;
    volSize.getBounds( xmin,ymin,zmin,xmax,ymax,zmax );

    float sizeX, sizeY, sizeZ;
    volSize.getSize( sizeX, sizeY, sizeZ );
    SbVec3f center = volSize.getCenter();

    SoDrawStyle *pStyle = new SoDrawStyle;
    pStyle->lineWidth = 2;
    SoLightModel *pLMod = new SoLightModel;
    pLMod->model = SoLightModel::BASE_COLOR;

    SoVertexProperty *pProp = new SoVertexProperty;
    pProp->vertex.setNum( 8 );
    pProp->vertex.set1Value( 0, xmin, ymin, zmin );
    pProp->vertex.set1Value( 1, xmax, ymin, zmin );
    pProp->vertex.set1Value( 2, xmax, ymax, zmin );
    pProp->vertex.set1Value( 3, xmin, ymax, zmin );
    pProp->vertex.set1Value( 4, xmin, ymin, zmax );
    pProp->vertex.set1Value( 5, xmax, ymin, zmax );
    pProp->vertex.set1Value( 6, xmax, ymax, zmax );
    pProp->vertex.set1Value( 7, xmin, ymax, zmax );
    pProp->orderedRGBA.set1Value( 0, SbColor(1,0,0).getPackedValue() );

    const int32_t indices[] = {
      0, 1, 2, 3, 0, -1,
        4, 5, 6, 7, 4, -1,
        0, 4, -1,
        1, 5, -1,
        2, 6, -1,
        3, 7, -1
    };
    SoIndexedLineSet *pLine = new SoIndexedLineSet;
    pLine->vertexProperty = pProp;
    pLine->coordIndex.setValues( 0, 24, indices );

    pBboxRoot->addChild( pStyle );
    pBboxRoot->addChild( pLMod  );
    pBboxRoot->addChild( pLine );
  }
}

///////////////////////////////////////////////////////////////////////
//
void updateVolAxesGeometry()
{
  if (m_volAxesSwitch == NULL)
    return;

  SbBox3f volSize = m_volData->extent.getValue();
  float xsize,ysize,zsize;
  volSize.getSize( xsize, ysize, zsize );
  SbVec3f center = volSize.getCenter();

  // Position axes at center of volume
  SoTranslation *pPos = new SoTranslation;
  pPos->translation = center;

  float coneHeight = (xsize / 20) + (ysize / 20) + (zsize / 20);
  coneHeight /= 3;

  SoSeparator *pXRoot =
    (SoSeparator*)SoNode::getByName("MISC_XGEOM");
  if (!pXRoot) {
    pXRoot = new SoSeparator;
    pXRoot->setName( "MISC_XGEOM" );
    m_volAxesSwitch->addChild( pXRoot );
  }
  else {
    pXRoot->removeAllChildren();
  }
  {
    SoMaterial *pMat = new SoMaterial;
    pMat->diffuseColor.setValue( SbColor(1,0,0) );
    SoRotation *pRot = new SoRotation;
    pRot->rotation    = SbRotation(SbVec3f(0,0,-1),PIO2);
    SoCylinder *pCyl = new SoCylinder;
    pCyl->height = xsize;
    pCyl->radius = ysize / 100;
    SoTranslation *pTran = new SoTranslation;
    pTran->translation = SbVec3f( 0,(xsize/2 + coneHeight/2),0 );
    SoCone *pCone = new SoCone;
    pCone->height = coneHeight; //xsize / 20;
    pCone->bottomRadius = ysize / 50;

    pXRoot->addChild( pPos );
    pXRoot->addChild( pMat );
    pXRoot->addChild( pRot );
    pXRoot->addChild( pCyl  );
    pXRoot->addChild( pTran );
    pXRoot->addChild( pCone );
  }

  SoSeparator *pYRoot =
    (SoSeparator*)SoNode::getByName("MISC_YGEOM");
  if (!pYRoot) {
    pYRoot = new SoSeparator;
    pYRoot->setName( "MISC_YGEOM" );
    m_volAxesSwitch->addChild( pYRoot );
  }
  else {
    pYRoot->removeAllChildren();
  }
  {
    SoMaterial *pMat = new SoMaterial;
    pMat->diffuseColor.setValue( SbColor(0,1,0) );
    SoCylinder *pCyl = new SoCylinder;
    pCyl->height = ysize;
    pCyl->radius = xsize / 100;
    SoTranslation *pTran = new SoTranslation;
    pTran->translation = SbVec3f( 0,(ysize/2 + coneHeight/2),0 );
    SoCone *pCone = new SoCone;
    pCone->height = coneHeight; //ysize / 20;
    pCone->bottomRadius = xsize / 50;

    pYRoot->addChild( pPos );
    pYRoot->addChild( pMat );
    pYRoot->addChild( pCyl );
    pYRoot->addChild( pTran );
    pYRoot->addChild( pCone );
  }

  SoSeparator *pZRoot =
    (SoSeparator*)SoNode::getByName("MISC_ZGEOM");
  if (!pZRoot) {
    pZRoot = new SoSeparator;
    pZRoot->setName( "MISC_ZGEOM" );
    m_volAxesSwitch->addChild( pZRoot );
  }
  else {
    pZRoot->removeAllChildren();
  }
  {
    SoMaterial *pMat = new SoMaterial;
    pMat->diffuseColor.setValue( SbColor(0,0,1) );
    SoRotation *pRot = new SoRotation;
    pRot->rotation = SbRotation(SbVec3f(1,0,0),PIO2);
    SoCylinder *pCyl = new SoCylinder;
    pCyl->height = zsize;
    pCyl->radius = ysize / 100;
    SoTranslation *pTran = new SoTranslation;
    pTran->translation = SbVec3f( 0,(zsize/2 + coneHeight/2),0 );
    SoCone *pCone = new SoCone;
    pCone->height = coneHeight; //zsize / 20;
    pCone->bottomRadius = ysize / 50;

    pZRoot->addChild( pPos );
    pZRoot->addChild( pMat );
    pZRoot->addChild( pRot );
    pZRoot->addChild( pCyl );
    pZRoot->addChild( pTran );
    pZRoot->addChild( pCone );
  }
}

///////////////////////////////////////////////////////////////////////
//
//For qsort
static int cmpInt(const void *a, const void *b)
{
  return (int)(*((int64_t *)a)-*((int64_t *)b));
}

///////////////////////////////////////////////////////////////////////
//
void createHistoLegend( SoGroup *pParent, int whichColormap )
{
  if (pParent == NULL)
    return;

  if (!m_showHistogram)
    return;

  // Legend (histogram)
  const float legXmin = -.885f;
  const float legXmax = -.78f;
  const float legYmin = -.8f;
  const float legYmax =  .8f;
  //    const float legFaceVerts1[][3] =
  //        {{legXmin,legYmin,0}, {legXmax,legYmin,0},
  //         {legXmax,legYmax,0}, {legXmin,legYmax,0}};
  //    const float legFaceVerts2[][3] =
  //        {{legXmin,legYmin,0.0002f}, {legXmax,legYmin,0.0002f},
  //         {legXmax,legYmax,0.0002f}, {legXmin,legYmax,0.0002f}};
  const float legLineVerts[][3] =
  {{legXmin,legYmin,0.0004f}, {legXmax,legYmin,0.0004f},
  {legXmax,legYmax,0.0004f}, {legXmin,legYmax,0.0004f}};
  const int32_t legLineIndex[] = {0, 1, 2, 3, 0};

  // LineSet to outline colormap
  SoVertexProperty *pLegProp = new SoVertexProperty;
  pLegProp->vertex.setValues( 0, 4, legLineVerts );
  pLegProp->orderedRGBA.set1Value( 0, 0xFFFFFFFF );
  SoIndexedLineSet *pLegLine = new SoIndexedLineSet;
  pLegLine->vertexProperty = pLegProp;
  pLegLine->coordIndex.setValues( 0, 5, legLineIndex );

  float offset = 0.004f;
  float histXmin = legXmin + offset;
  float histXmax = legXmax - offset;
  float histYmin = legYmin + offset;
  float histYmax = legYmax - offset;
  float histLength = histYmax - histYmin;
  float histHeight = histXmax - histXmin;

  // Pull the legend out close to the near clip plane
  // (Note this depends on how the app setup the camera...)
  if(!m_histoTrans){
    m_histoTrans = new SoTranslation;
    m_histoTrans->translation.setValue( SbVec3f(0,0,.499f) );
  }

  SoDrawStyle *pStyle = new SoDrawStyle;
  pStyle->lineWidth = 2;

  int i;
  int numBins;
  int64_t *hvalues; //has to pass int64_t for ldm volumes.
  //this version of volData->getHistogram works for each format

  // Get histogram of data and find max value
  // (values outside the opaque range are ignored)
  SbBool ok = m_volData->getHistogram ( numBins, hvalues );
  if(!ok)return;

  // Convert the histogram into a 256 entries histogram
  int64_t values[256];
  {
     SoVolumeData::DataType dataType = m_volData->getDataType();
    bool dataIsFloat  = m_volData->isDataFloat( dataType );
    if (! dataIsFloat) {
      bool dataIsSigned = m_volData->isDataSigned( dataType );
      int  dataSise     = m_volData->getDataSize();
      int  numSigBits   = m_volData->numSigBits();
      if (dataSise == 4)
        numSigBits /= 2;
      if (dataIsSigned) {
#ifdef _WIN64
        hvalues = hvalues + numBins/2 - (1i64<<(numSigBits-1));
#else
        //hvalues = hvalues + numBins/2 - (1<<(numSigBits-1));
#endif
      }
      numBins = (1 << numSigBits);
    }
    if (numBins >= 256) {
      int r = numBins / 256;
      for (int i = 0; i < 256; i++) {
        int64_t sum = 0;
        for (int j = 0; j < r; j++)
          sum += hvalues[r*i+j];
        values[i] = sum;
      }
    }
    else {
      int r = 256 / numBins;
      for (int i = 0; i < numBins; i++) {
        for (int j = 0; j < r; j++)
          values[r*i+j] = hvalues[i];
      }
    }
    numBins = 256;
  }

  int minColormap, maxColormap, minOpaqueMap, maxOpaqueMap;
  SbBool invertTransparency;
  switch (whichColormap) {
  case 0: // volume
    minColormap  = m_minColorMap;
    maxColormap  = m_maxColorMap;
    minOpaqueMap = m_minOpaqueMap;
    maxOpaqueMap = m_maxOpaqueMap;
    invertTransparency = m_invertTransparency;
    break;
  case 1: // slices
    minColormap  = m_minSliceColorMap;
    maxColormap  = m_maxSliceColorMap;
    minOpaqueMap = m_minSliceOpaqueMap;
    maxOpaqueMap = m_maxSliceOpaqueMap;
    invertTransparency = m_invertSliceTransparency;
    break;
  }

  // Find max value
  int64_t maxVal = 0;
  int     maxBin = -1;
  for (i = 0; i < numBins; i++) {
    if (!(minColormap <= i && i <= maxColormap))
      continue;
    if ((minOpaqueMap <= i && i <= maxOpaqueMap) == invertTransparency)
      continue;
    if (values[i] > maxVal) {
      maxVal = values[i];
      maxBin = i;
    }
  }

  // Find max value again, ignoring the largest value (probably a spike)
  maxVal = 0;
  for (i = 0; i < numBins; i++) {
    if (!(minColormap <= i && i <= maxColormap))
      continue;
    if ((minOpaqueMap <= i && i <= maxOpaqueMap) == invertTransparency)
      continue;
    if (i == maxBin)
      continue;
    if (values[i] > maxVal)
      maxVal = values[i];
  }

  SoLineSet *pBinLines = new SoLineSet;
  // If maxVal is zero, then we don't actually have any data
  // (possibly could not open input file), so don't draw a
  // bunch of garbage in the histogram.       --mmh Dec-2000
  if (maxVal > 0)
  {
    SoVertexProperty *pBinProp = new SoVertexProperty;
    pBinProp->vertex.setNum( 2 * numBins );
    pBinProp->orderedRGBA.set1Value( 0, 0xFF6F2FFF );
    pBinLines->vertexProperty = pBinProp;
    pBinLines->numVertices.setNum( numBins );
    int vertIndex  = 0;
    int indexIndex = 0;

    int32_t*  _numVertices = pBinLines->numVertices.startEditing();
    SbVec3f* _vertex = pBinProp->vertex.startEditing();
    for (i = 0; i < numBins; i++)
    {
      float y = histYmin + i * (histLength/numBins);
      _vertex[vertIndex] = SbVec3f( histXmax, y, 0 );
      vertIndex++;
      // Note values below opaque range are ignored.
      // Do NOT modify the contents of the values array!
      // Any changes are permanent (until next volume loaded)
      float val;
      if ((minOpaqueMap <= i && i <= maxOpaqueMap) == invertTransparency)
        val = 0;
      //else if (!(minColormap <= i && i <= maxColormap))
      //  val = 0;
      else
        val = (float)((values[i] > maxVal) ? maxVal : values[i]);

      float x = histXmax - ((val/(float)maxVal)*histHeight);
      _vertex[vertIndex] = SbVec3f( x, y, 0 );
      _numVertices[indexIndex++] = 2 ;
      vertIndex++;
    }
    pBinLines->numVertices.finishEditing();
    pBinProp->vertex.finishEditing();

  }

  // Build the legend scene graph
  SoSeparator *pLegSep = new SoSeparator;
  pLegSep->addChild( m_histoTrans );
  pLegSep->addChild( pLegLine );
  pLegSep->addChild( pStyle );
  pLegSep->addChild( pBinLines );

  if (pParent->getNumChildren())
    pParent->replaceChild( 0, pLegSep );
  else
    pParent->addChild( pLegSep );

  //Find the median value and use it as a default value for isosurface rendering
  int64_t tmp[256];
  memcpy(tmp, values, sizeof(tmp));
  qsort(tmp, numBins, sizeof(values[0]), cmpInt);
  int64_t median = values[numBins/2];
  for(i = 0; i < numBins; i++) {
    if(values[i] == median) {
      m_posMaxValueHisto = i;
      break;
    }
  }

  return;
}//

///////////////////////////////////////////////////////////////////////
//
// Handle key release events to hack in temporary functionality
//
#if defined(WIN32) && !defined(CAVELIB)

#include <Inventor/SoOffscreenRenderer.h>

SbBool EventCB( void *userData, MSG *msg )
{
  if(msg->message == WM_SIZE){

    SbVec2s size = m_viewer->getSize();
    float min = size[0] < size[1] ? size[0] : size[1];
    float w = (float)size[0]/min;
    float h = (float)size[1]/min;


    if(m_colorMapTrans)
      m_colorMapTrans->translation.setValue( SbVec3f(1-w, 1-h, .499f) );
    if(m_loadingSignTrans)
      m_loadingSignTrans->translation.setValue( SbVec3f(w - .1f , h - .1f, .5f) );
    if(m_histoTrans)
      m_histoTrans->translation.setValue( SbVec3f(1-w, 1-h, .499f) );

  }
  if (msg->message == WM_KEYUP) {
    int key = (int)(msg->wParam);
    sprintf( buffer, "WM_KEYUP: %c (%d)\n", key, key );
    OutputDebugString( buffer );

    if (key == 'A')
    {
       // Antialiasing (for production screen shots)
      float antialiasingQuality = m_viewer->getAntialiasingQuality();
      if (antialiasingQuality == 0.0)
      {
        if (GetKeyState( VK_SHIFT ) < 0) // high-order bit set means key is down
          antialiasingQuality = 1.0f;
        else
          antialiasingQuality = 0.5f;
      }
      else
        antialiasingQuality = 0.0f; // disable
      m_viewer->setAntialiasing(antialiasingQuality);
      m_viewer->scheduleRedraw();
    }
    if (key == 'D') {   // Toggle visibility of dialog boxes
#ifndef NO_DIALOG
      /*if (m_dialogBox->isVisible())
      m_dialogBox->hide();
      else
      m_dialogBox->show();
      if (m_dialogBox2->isVisible())
      m_dialogBox2->hide();
      else
      m_dialogBox2->show();*/
#endif
    }
    if (key == 'F') {   // Framerate
      if (m_showFrameRate) {
        m_showFrameRate = FALSE;
        m_viewer->setFramesPerSecondCallback( NULL, NULL );
      }
      else {
        m_showFrameRate = TRUE;
        m_viewer->setNumSamples( DEF_NUM_SAMPLES );
        m_viewer->setFramesPerSecondCallback( viewerFPSCB, (void*)myWindow );
      }
    }
    if (key == 'P')
    {
       // Print
      float antiAliasingQuality = m_viewer->getAntialiasingQuality();

      // By default snapshot is same size as current viewer window
      SbViewportRegion vport = m_viewer->getViewportRegion();
      const char *str = SoPreferences::getValue( "VOLREND_SnapshotSize" );
      if (str) {
        int w, h;
        int nitems = sscanf( str, "%d %d", &w, &h );
        if (nitems == 2)
          //vport.setViewportPixels( 0, 0, w, h );
          vport.setWindowSize( w, h );
      }

      SoGLRenderAction action( vport );
      action.setTransparencyType(SoGLRenderAction::BLEND);

      SoOffscreenRenderer offscreen( vport );
      offscreen.setGLRenderAction( &action );
      SoNode *pScene = m_viewer->getSceneManager()->getSceneGraph();
      m_viewer->getSceneManager()->setAntialiasing(antiAliasingQuality);

      // Request sharing of display lists and texture objects with
      // the offscreen context -- required until VolumeVizLDM is
      // smarter about managing texture ids and contexts.
      offscreen.setShareContext( m_viewer->getShareContext() );

      SbBool ok = offscreen.render( pScene );
      if (ok) {
        FILE *fp = fopen( "offscreen.bmp", "w" );
        offscreen.writeToBMP( fp );
        fclose( fp );
        OutputDebugString( "OffscreenRender OK\n" );
      }
      else
        SoDebugError::postWarning("Unable to Print","OffscreenRender failed");
    }
    if (key == 'R') {   // Redraw
      SoXtViewer *pViewer = (SoXtViewer*)userData;
      pViewer->scheduleRedraw();
    }
    if (key == 'X') {   // X axis view
      SoCamera *pCam = m_viewer->getCamera();
      if (GetKeyState( VK_SHIFT ) < 0) {    // high-order bit set means key is down
        pCam->position = SbVec3f(-1,0,0);
        pCam->orientation.setValue( SbVec3f(0,1,0), -PIO2 );
      }
      else {
        pCam->position = SbVec3f(1,0,0);
        pCam->orientation.setValue( SbVec3f(0,1,0), PIO2 );
      }
      m_viewer->viewAll();
    }
    if (key == 'Y') {   // Y axis view
      SoCamera *pCam = m_viewer->getCamera();
      if (GetKeyState( VK_SHIFT ) < 0) {    // high-order bit set means key is down
        pCam->position = SbVec3f(0,-1,0);
        pCam->orientation.setValue( SbVec3f(1,0,0), PIO2 );
      }
      if (key == 'F') {   // Framerate
        if (m_showFrameRate) {
          m_showFrameRate = FALSE;
          m_viewer->setFramesPerSecondCallback( NULL, NULL );
        }
        else {
          m_showFrameRate = TRUE;
          m_viewer->setNumSamples( DEF_NUM_SAMPLES );
          m_viewer->setFramesPerSecondCallback( viewerFPSCB, (void*)myWindow );
        }
      }
      if (key == 'P')
      {
          // Print
          float antiAliasingQuality = m_viewer->getAntialiasingQuality();

          SoGLRenderAction action( m_viewer->getViewportRegion() );
          action.setTransparencyType(SoGLRenderAction::BLEND);

          SoOffscreenRenderer offscreen( m_viewer->getViewportRegion() );
          offscreen.setGLRenderAction( &action );
          SoNode *pScene = m_viewer->getSceneManager()->getSceneGraph();
          m_viewer->getSceneManager()->setAntialiasing(antiAliasingQuality);

          // Request sharing of display lists and texture objects with
          // the offscreen context -- required until VolumeVizLDM is
          // smarter about managing texture ids and contexts.
          offscreen.setShareContext( m_viewer->getShareContext() );

          SbBool ok = offscreen.render( pScene );
          if (ok) {
              FILE *fp = fopen( "offscreen.bmp", "w" );
              offscreen.writeToBMP( fp );
              fclose( fp );
              OutputDebugString( "OffscreenRender OK\n" );
          }
          else
            SoDebugError::postWarning("Unable to Print","OffscreenRender failed");
      }
      if (key == 'R') {   // Redraw
          SoXtViewer *pViewer = (SoXtViewer*)userData;
          pViewer->scheduleRedraw();
      }
      if (key == 'X') {   // X axis view
          SoCamera *pCam = m_viewer->getCamera();
          if (GetKeyState( VK_SHIFT ) < 0) {    // high-order bit set means key is down
              pCam->position = SbVec3f(-1,0,0);
              pCam->orientation.setValue( SbVec3f(0,1,0), -PIO2 );
          }
          else {
              pCam->position = SbVec3f(1,0,0);
              pCam->orientation.setValue( SbVec3f(0,1,0), PIO2 );
          }
          m_viewer->viewAll();
      }
      m_viewer->viewAll();
    }
    if (key == 'Z') {   // Z axis view
      SoCamera *pCam = m_viewer->getCamera();
      if (GetKeyState( VK_SHIFT ) < 0) {    // high-order bit set means key is down
        pCam->position = SbVec3f(0,0,-1);
        pCam->orientation.setValue( SbVec3f(0,1,0), -(PIO2*2) );
      }
      else {
        pCam->position = SbVec3f(0,0,1);
        pCam->orientation.setValue( SbVec3f(1,0,0), 0 );
      }
      m_viewer->viewAll();
    }

      if (key == 'W') {
        // Write to file with alternateRep enabled.
        // (don't write headlight, viewer will set up its own when file is read)
        SoVolumeRendering::setWriteAlternateRep( TRUE );
        SoWriteAction wa;
        SbBool isHL = m_viewer->isHeadlight();
        m_viewer->setHeadlight( FALSE );
        wa.getOutput()->openFile( "test.iv" );
        wa.apply( m_root );
        wa.getOutput()->closeFile();
        m_viewer->setHeadlight( isHL );
      }
      //if (key == 'A') {
        //initDialog();
        //printf("init\n");
      //}

  static int change = 0;
        if (key == 'B') {
				  int wDivide = 1;
				  int hDivide = 1;
				  int zDivide = 5;
				  int wValue = m_width/wDivide;
				  int hValue = m_height/hDivide;
				  int dValue = m_depth/zDivide;
          //m_volROI->box.setValue( 0,0,0, volWidth/10, volHeight/10, volDepth/10 );
          switch (change)
          {
          case 0:
            m_ROIManip->subVolume.setValue( 0,0,0, wValue, hValue, dValue);
            break;
          case 1:
            m_ROIManip->subVolume.setValue(0, 0, 2*dValue, wValue, hValue, 3*dValue );
					  break;
          case 2:
            m_ROIManip->subVolume.setValue(0, 0, 4*dValue, wValue, hValue, 5*dValue );
            break;
          }
          change++;
          if(change==3)change=0;
          //initDialog();
          //printf("init\n");
        }
        if (key == 'V') {
				  int wDivide = 1;
				  int hDivide = 1;
				  int zDivide = 5;
				  int wValue = m_width/wDivide;
				  int hValue = m_height/hDivide;
				  int dValue = m_depth/zDivide;
          //m_volROI->box.setValue( 0,0,0, volWidth/10, volHeight/10, volDepth/10 );
          switch (change)
          {
          case 0:
            m_ROIManip->box.setValue( 0,0,0, wValue, hValue, dValue);
            break;
          case 1:
            m_ROIManip->box.setValue(0, 0, 2*dValue, wValue, hValue, 3*dValue );
					  break;
          case 2:
            m_ROIManip->box.setValue(0, 0, 4*dValue, wValue, hValue, 5*dValue );
            break;
          }
          change++;
          if(change==3)change=0;
          //initDialog();
          //printf("init\n");
        }
        if (key == 'I') {
          double angle = 0;
          SbElapsedTime timer;
          timer.reset();
          for(int i = 0;i<100;i++){
            angle+=PI/50.;
            m_rotation->rotation.setValue(SbVec3f( 0, 1, 0), (float)angle);
            m_viewer->render();
          }
#if defined(_WIN32) && !defined(SOQT)
          char buf[256];
          sprintf( buf, "Render time = %.3f seconds (100 frames)\n",timer.getElapsed());
          SbString str(buf) ;
          SoConsole* console = SoXt::getErrorConsole();
          console->printMessage(str);
#else
      printf("Render time = %.3f\n",timer.getElapsed());
#endif
    }
    if (key == 'U') {
      const SbBox3i32 reg(0,0,0,m_width, m_height, m_depth);
      m_volData->updateRegions( &reg, 1);
    }
  }

  // Note: For arrow keys we have to handle them on the KEYDOWN event
  //       to prevent the viewer from processing them...
  if (msg->message == WM_KEYDOWN) {
    int key = (int)(msg->wParam);
    sprintf( buffer, "WM_KEYDOWN: %c (%d)\n", key, key );
    OutputDebugString( buffer );

    if (key == VK_UP) { // Up Arrow key
      // Increment slice number for current orthoslice
      float x,y,z,xfrac,yfrac,zfrac;
      m_draggerSlicePos.getValue( x, y, z );
      int value = m_orthoTab[m_currentSlice]->sliceNumber.getValue();
      int axe = m_orthoTab[m_currentSlice]->axis.getValue();
      value++;
      if (axe==SoOrthoSlice::X) {
        if (value > m_width - 1) value = m_width - 1;
        xfrac = (float)value / (float)(m_width - 1);
        x = m_minWidth + xfrac * (m_maxWidth - m_minWidth);
      }
      else if (axe==SoOrthoSlice::Y){
        if (value > m_height - 1) value = m_height - 1;
        yfrac = (float)value / (float)(m_height - 1);
        y = m_minHeight + yfrac * (m_maxHeight - m_minHeight);
      }
      else{
        if (value > m_depth - 1) value = m_depth - 1;
        zfrac = (float)value / (float)(m_depth - 1);
        z = m_minDepth + zfrac * (m_maxDepth - m_minDepth);
      }
      m_orthoTab[m_currentSlice]->sliceNumber = value;
      m_draggerSlicePos.setValue( x, y, z );
      m_draggerVolRender->enableValueChangedCallbacks( FALSE );
      m_draggerVolRender->translation.setValue( x, y, z );
      m_draggerVolRender->enableValueChangedCallbacks( TRUE );
#ifndef CAVELIB
      SoDialogIntegerSlider* sliceSlider = (SoDialogIntegerSlider*)myTopLevelDialog->searchForAuditorId(SbString("slicenumber"));
      sliceSlider->value.setValue(value);
#endif
      return TRUE;
    }

    if (key == VK_DOWN) { // Down Arrow key
      // Decrement slice number for current orthoslice
      int value = m_orthoTab[m_currentSlice]->sliceNumber.getValue() - 1;
      if (value < 0)
        return TRUE; // Can't go any farther
      float x,y,z,xfrac,yfrac,zfrac;
      m_draggerSlicePos.getValue( x, y, z );
      int axe = m_orthoTab[m_currentSlice]->axis.getValue();
      if (axe==SoOrthoSlice::X) {
        xfrac = (float)value / (float)(m_width - 1);
        x = m_minWidth + xfrac * (m_maxWidth - m_minWidth);
      }
      else if (axe==SoOrthoSlice::Y){
        yfrac = (float)value / (float)(m_height - 1);
        y = m_minHeight + yfrac * (m_maxHeight - m_minHeight);
      }
      else{
        zfrac = (float)value / (float)(m_depth - 1);
        z = m_minDepth + zfrac * (m_maxDepth - m_minDepth);
      }
      m_orthoTab[m_currentSlice]->sliceNumber = value;
      m_draggerSlicePos.setValue( x, y, z );
      m_draggerVolRender->enableValueChangedCallbacks( FALSE );
      m_draggerVolRender->translation.setValue( x, y, z );
      m_draggerVolRender->enableValueChangedCallbacks( TRUE );
#ifndef CAVELIB
      SoDialogIntegerSlider* sliceSlider = (SoDialogIntegerSlider*)myTopLevelDialog->searchForAuditorId(SbString("slicenumber"));
      sliceSlider->value.setValue(value);
#endif
      return TRUE;
    }
  }
  return FALSE;
}
#endif

////////////////////////////////////////////////////////////////////////

#ifdef CAVELIB
void keyEventCB( void * /*userData*/, SoEventCallback * )
{
#else
void keyEventCB( void * /*userData*/, SoEventCallback *node )
{

  const SoEvent *pEvent = node->getEvent();

  // Key 'F' toggles display of frame rate
  if (SoKeyboardEvent::isKeyPressEvent( pEvent, SoKeyboardEvent::F)) {
    if (m_showFrameRate) {
      m_showFrameRate = FALSE;
      m_viewer->setFramesPerSecondCallback( NULL, NULL );
    }
    else {
      m_showFrameRate = TRUE;
      m_viewer->setNumSamples( DEF_NUM_SAMPLES );
      m_viewer->setFramesPerSecondCallback( viewerFPSCB, (void*)myWindow );
    }
  }
  // Key 'R' requests a redraw
  if (SoKeyboardEvent::isKeyPressEvent( pEvent, SoKeyboardEvent::R)) {
    m_viewer->scheduleRedraw();
  }
#endif // CAVELIB
}

////////////////////////////////////////////////////////////////////////

#ifndef CAVELIB

void viewerFPSCB( float fps, void *userData, SoXtViewer *viewer )
{
  static char buf[80];
#if defined(_DEBUG)
	int curNumFrames = viewer->getNumSamples();
	sprintf( buf, "OIV Volume Rendering - fps: %.2f frames: %d",
		fps, curNumFrames );
#else
  sprintf( buf, "OIV Volume Rendering - fps: %.2f", fps );
#endif


#ifdef WIN32
  HWND hwnd = (HWND)userData;
  SetWindowText( hwnd, buf );
#elif defined(__APPLE__)
#else
  Widget widget = (Widget)userData;
  XSetStandardProperties(SoXt::getDisplay(), XtWindow(widget), buf, NULL, None, NULL, 0, NULL);
  XFlush( SoXt::getDisplay() );
#endif

  // We'd like to sample the frame rate every 2-3 seconds,
  // so we don't decrease the frame rate by displaying it. :-)
  int numFrames = (int)(fps * 2.5);
  if (numFrames < 1) numFrames = 1;
  else if (numFrames > 100) numFrames = 100;
  viewer->setNumSamples( numFrames );
}

void
viewerStartCB( void * /*userData*/, SoXtViewer * /*viewer*/ )
{
  // Called when user begins viewer interaction
  // We use this flag to disable front buffer rendering
  isInteracting = 1;
}

void
viewerFinishCB( void * /*userData*/, SoXtViewer *viewer )
{
  // Called when user finishs viewer interaction
  // Unsetting this flag re-enables front buffer rendering.
  // Schedule a redraw to be sure the volume is re-rendered.
  isInteracting = 0;
  if (isFrontBufRendering) {
    viewer->scheduleRedraw();
  }
}

#endif //CAVELIB

#ifndef CAVELIB
void
fpsCB(float fps, void *, SoXtViewer *viewer) {
  SbString titleStr("VolumeViz : ") ;
  SbString fpsStr = SbString().setNum( (int)fps );
  titleStr += fpsStr ;
  titleStr += " fps" ;
  viewer->setTitle(titleStr) ;
}
#endif //CAVELIB

///////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
//
void updateColorMapLegend( SoTransferFunction *pXferNode,
                          SoTexture2 *pTexNode )
{
  // Allocate a 256x1 4-component texture image for colormap
  //
  // TODO: Make this more general (won't always be 256 values!)
  unsigned int *pRGBA;
  int numEntries = pXferNode->getPackedColorMap( pRGBA, 0 );
  if (numEntries == 0)
    return;
  int i;
  unsigned char *pImage = (unsigned char *)malloc( 256 * 4 );
  unsigned char *p1 = pImage;
  for (i = 0; i < 256; i++) {
    unsigned char *rgba = (unsigned char*) pRGBA++;
    *p1++ = rgba[0];
    *p1++ = rgba[1];
    *p1++ = rgba[2];
    *p1++ = rgba[3];
  }
  pTexNode->image.setValue( SbVec2s(256,1), 4, pImage );
  free( pImage );
}

////////////////////////////////////////////////////////////////////////
//
void colorMapSensorCB( void *data, SoSensor *sensor )
{
  SoNodeSensor *pNodeSensor = (SoNodeSensor*)sensor;
  SoTransferFunction *pXferNode =
    (SoTransferFunction*)pNodeSensor->getAttachedNode();
  SoTexture2 *pTexNode = (SoTexture2*)data;
  if (pXferNode)
    updateColorMapLegend( pXferNode, pTexNode );
}
//----

#ifndef CAVELIB

///////////////////////////////////////////////////////////////////////
// This routine is called for every mouse movement.
void
mouseMoveEventCB(void *userData, SoEventCallback *eventCB)
{

	SoSeparator *root = (SoSeparator *) userData;
	const SoEvent *event = eventCB->getEvent();

	SoPickedPoint *picked_point;

	static SoLineSet *myProfileLineSet;
	static SbBool flagVolumeRender = FALSE;
	static SbVec3f profile[2];
	static SoVolumeRenderDetail *volumeRenderDetail;

   if( m_leftMousePressed ) {

	   // SoRayPickAction to get point in the scene graph.
		SoRayPickAction picked_action = SoRayPickAction( m_viewer->getViewportRegion());
		SbVec2s myVec2s = event->getPosition();
		picked_action.setPoint( myVec2s );
		picked_action.apply( root);
		picked_point = picked_action.getPickedPoint(0);
		if( picked_point ) { // There is something.
			SbVec3f myVec = picked_point->getPoint();
			float x, y, z;
			myVec.getValue(x,y,z);
			SoNode* selected_node = (picked_point->getPath())->getTail();
			static SoSeparator *profileRoot;
			profileRoot = m_profile->getProfileRoot();

			if( selected_node->isOfType( SoVolumeRender::getClassTypeId() ) ) {
                SbVec3i32 dataProfilePos[2];
				for (int i = ((const SoFullPath *) picked_point->getPath())->getLength() - 1; i >= 0; i--) {
				}
				volumeRenderDetail = (SoVolumeRenderDetail *)picked_point->getDetail();
				int numValues =  volumeRenderDetail->getProfileDataPos(dataProfilePos);
				volumeRenderDetail->getProfileObjectPos(profile);

				static int flag = 0;
				static SoVertexProperty *myProfileVertexProperty;
				static double min=0, max=255;

				if( !flag ) {
					// The profile
					myProfileVertexProperty = new SoVertexProperty();
					myProfileLineSet = new SoLineSet();
					myProfileLineSet->vertexProperty = myProfileVertexProperty;
					myProfileVertexProperty->orderedRGBA.setValue(0x00FF00FF);
					profileRoot->addChild(myProfileLineSet);
					if (m_showHistogram)
          {
            m_volData->getMinMax(min,max);
          }
        }

        SbVec3i32 *pos    = new SbVec3i32[numValues];
        SbVec3f   *objPos = new SbVec3f[numValues];

        SbVec3f *curve = new SbVec3f[numValues];
        for( int j=0; j<numValues; j++ ) {
          float yValue = (float)volumeRenderDetail->getProfileValue(j, pos[j], &objPos[j]);
          if( yValue > 255 )
            curve[j].setValue((float)j, yValue/255, 1.5f);	// 16 Bits.
          else
            curve[j].setValue((float)j, yValue, 1.5f);		// 8 Bits
        }
        myProfileLineSet->numVertices = numValues;
        myProfileLineSet->startIndex = 0;
        myProfileVertexProperty->vertex.setValues( 0, numValues, curve );
        if( !flag && m_newFile ) {
          m_profile->profileViewAll();
          flag++;
        }
        flagVolumeRender = TRUE;

        static SoText2 *volText2 = NULL;
        if( !volText2 ) {
          SoSeparator *textSep = m_profile->getTextProfileRoot();

          SoSeparator *localSep = new SoSeparator();
          textSep->addChild(localSep);
          SoFont  *pFont = new SoFont;
          SoTranslation *pTran = new SoTranslation;
          pTran->translation = SbVec3f( -.9f, 0.0f, 0.49f );
          pFont->size = 12;
          localSep->addChild(pTran);
          localSep->addChild(pFont);
          volText2 = new SoText2();
          localSep->addChild(volText2);
        }

        SbVec3i32 dataPos;
        SbVec3f   geomPos;
        if(m_volData->getDataType() == SoVolumeData::FLOAT){
          double   dataVal;
          SbBool haveData = volumeRenderDetail->
            getFirstNonTransparentValue( dataVal, dataPos, &geomPos );
          if (haveData) {
            char buffer[255];
            sprintf( buffer, "Volume = %.3f at (%d,%d,%d)",
              dataVal, dataPos[0], dataPos[1], dataPos[2] );
            volText2->string.setValue(buffer);
          }
          else
            volText2->string.setValue( "Volume = <transparent>" );

        }
        else{
          int64_t   dataVal;
          SbBool haveData = volumeRenderDetail->
            getFirstNonTransparentValue( dataVal, dataPos, &geomPos );
          if (haveData) {
            sprintf( buffer, "Volume = %3d at (%d,%d,%d)",
                     (int)dataVal, dataPos[0], dataPos[1], dataPos[2] );
            volText2->string.setValue(buffer);
          }
          else
            volText2->string.setValue( "Volume = <transparent>" );
        }
      }
      char buffer[256];
      if( selected_node->isOfType( SoOrthoSlice::getClassTypeId() ) ) {
        SoOrthoSliceDetail *orthoSliceDetail = (SoOrthoSliceDetail *)picked_point->getDetail();
        static int flag = 0;

        static SoText2 *orthoText2;
        if( !flag ) {
          SoSeparator *textSep = m_profile->getTextProfileRoot();

          SoSeparator *localSep = new SoSeparator();
          textSep->addChild(localSep);
          SoFont  *pFont = new SoFont;
          SoTranslation *pTran = new SoTranslation;
          pTran->translation = SbVec3f( -.9f, -.8f, 0.49f );
          pFont->size = 12;
          localSep->addChild(pTran);
          localSep->addChild(pFont);
          orthoText2 = new SoText2();
          localSep->addChild(orthoText2);
          flag++;
        }
        SbVec3i32 pos = orthoSliceDetail->getValueDataPos();
        if(m_volData->getDataType() == SoVolumeData::FLOAT)
          sprintf( buffer, "Ortho Slice = %.3f at (%d,%d,%d)",
          orthoSliceDetail->getValueD(), pos[0], pos[1], pos[2] );
        else
          sprintf( buffer, "Ortho Slice = %3d at (%d,%d,%d)",
          (int)orthoSliceDetail->getValue(), pos[0], pos[1], pos[2] );

        orthoText2->string.setValue(buffer);
      }
      if( selected_node->isOfType( SoObliqueSlice::getClassTypeId() ) ) {
        SoObliqueSliceDetail *obliqueSliceDetail = (SoObliqueSliceDetail *)picked_point->getDetail();
        static int flag = 0;

        static SoText2 *obliqueText2;
        if( !flag ) {
          SoSeparator *textSep = m_profile->getTextProfileRoot();
          SoSeparator *localSep = new SoSeparator();
          textSep->addChild(localSep);
          SoFont  *pFont = new SoFont;
          SoTranslation *pTran = new SoTranslation;
          pTran->translation = SbVec3f( -0.9f, 0.8f, 0.49f );
          pFont->size = 12;
          localSep->addChild(pTran);
          localSep->addChild(pFont);
          obliqueText2 = new SoText2();
          localSep->addChild(obliqueText2);
          flag++;
        }
        SbVec3i32 pos = obliqueSliceDetail->getValueDataPos();
        if(m_volData->getDataType() == SoVolumeData::FLOAT)
          sprintf( buffer, "Ortho Slice = %.3f at (%d,%d,%d)",
          obliqueSliceDetail->getValueD(), pos[0], pos[1], pos[2] );
        else
          sprintf( buffer, "Oblique Slice = %3d at (%d,%d,%d)",
          (int)obliqueSliceDetail->getValue(), pos[0], pos[1], pos[2] );

        obliqueText2->string.setValue(buffer);
      }
    }
  } else {
    // Let's not be changing the scene graph (and triggering a long, slow
    // redraw!) if we're doing front buffer rendering...     mmh Dec-2000
    if( m_leftMouseReleased && flagVolumeRender && !isFrontBufRendering  ) {
      static SoVertexProperty *myTraceVertexProperty;
      static int compt = 0;
      flagVolumeRender = FALSE;
      if( !compt /*|| m_newFile */) {
        m_textSwitch = new SoSwitch();
        m_textSwitch->whichChild = SO_SWITCH_ALL;
        m_myLineSet = new SoLineSet();
        m_textSwitch->addChild(m_myLineSet);
        root->insertChild(m_textSwitch, 1);
        myTraceVertexProperty = new SoVertexProperty();
        myTraceVertexProperty->orderedRGBA.setValue(0xFFFF00FF);
        m_myLineSet->vertexProperty = myTraceVertexProperty;
        m_newFile = FALSE;
        compt++;
      }
      if( m_newFile ) m_textSwitch->whichChild = SO_SWITCH_ALL;
      myTraceVertexProperty->vertex.setValues( 0, 2, profile );
      m_myLineSet->numVertices = 2;
      m_myLineSet->startIndex = 0;
    }
  }
}
///////////////////////////////////////////////////////////////////////
// This routine is called for every mouse movement.
void
mouseKeyEventCB(void *userData, SoEventCallback *eventCB)
{
    static SbVec2s downPos(0,0);
//	SoSeparator *root = (SoSeparator *) userData;
	const SoEvent *event = eventCB->getEvent();

	// Check for mouse button being pressed
	if (SO_MOUSE_PRESS_EVENT(event, ANY)) {
		m_leftMousePressed = TRUE;
        downPos = event->getPosition();
		eventCB->setHandled();
#ifdef WIN32
                OutputDebugString( "mouseKeyEventCB handled button press\n" );
#endif
  } else {
    if( SO_MOUSE_RELEASE_EVENT(event, ANY)) {
      m_leftMouseReleased = TRUE;
    } else {
      m_leftMouseReleased = FALSE;
    }

    // If user just clicked, then no motion event occured, but we
    // still want to pick (at least on slices).      mmh Dec-2000
    // Do this *before* resetting m_leftMousePressed...
    SbVec2s upPos = event->getPosition();
    if (upPos == downPos) {
      mouseMoveEventCB( userData, eventCB );
    }

    m_leftMousePressed = FALSE;
  }
}

#endif // CAVELIB

///////////////////////////////////////////////////////////////////////
//=========================================================================================
// Volume Rendering dragger
void
setDraggerSlicePos (const SbVec3f &pos) {
  // set the dragger position
  m_draggerSlicePos = pos;
  m_draggerVolRender->enableValueChangedCallbacks( FALSE );
  m_draggerVolRender->translation.setValue(SbVec3f(0,0,0));
  m_draggerVolRender->rotation = SbRotation(SbVec3f(0,1,0),m_draggerNormal);
  m_draggerVolRender->scaleFactor.setValue(m_scaleFactor,m_scaleFactor,m_scaleFactor);
  m_draggerVolRender->enableValueChangedCallbacks( TRUE );
}

////////////////////////////////////////////////////////////////////////

void initDialog(void)
{
#ifndef CAVELIB

  //createOrthoSlices(1, SO_SWITCH_ALL);
  //m_switchOrthoTab[m_currentSlice]->whichChild = SO_SWITCH_NONE;
  //SoDialogCheckBox* orthoVis = (SoDialogCheckBox*)myTopLevelDialog->searchForAuditorId(SbString("orthovisibility"));
  //orthoVis->state = FALSE;

  //SoRowDialog* row = (SoRowDialog*)myTopLevelDialog->searchForAuditorId(SbString("pagingcontrol"));
  //row->enable = TRUE;
  // get volume dimension
  SbVec3i32 dimension = m_volData->data.getSize();

  int m_tileSize = 64;

  // compute level max
  int maxDim = dimension[0];
  int levelMax = 0;
  if (dimension[1] > maxDim) maxDim = dimension[1];
  if (dimension[2] > maxDim) maxDim = dimension[2];
  for (levelMax = 0; (1<<levelMax)*m_tileSize < maxDim; levelMax++) ;
  if(m_volData->getReader() )
  {
    SoDialogComboBox* minres = (SoDialogComboBox*)myTopLevelDialog->searchForAuditorId(SbString("resthreshold"));
    minres->selectedItem = levelMax-1;
  }
  m_levelMax = levelMax;

  m_volRenderObliSliceSwitch->whichChild = SO_SWITCH_NONE;
  SoDialogCheckBox* obliqueVis = (SoDialogCheckBox*)myTopLevelDialog->searchForAuditorId(SbString("obliquevisibility"));
  obliqueVis->state = FALSE;

  SoDialogIntegerSlider* colorMapMax = (SoDialogIntegerSlider*)myTopLevelDialog->searchForAuditorId(SbString("colormap max"));
  colorMapMax->max.setValue(255);
  colorMapMax->min.setValue(0);
  colorMapMax->value.setValue(m_maxColorMap);
  SoDialogIntegerSlider* colorMapMin = (SoDialogIntegerSlider*)myTopLevelDialog->searchForAuditorId(SbString("colormap min"));
  colorMapMin->max.setValue(255);
  colorMapMin->min.setValue(0);
  colorMapMin->value.setValue(m_minColorMap);
  SoDialogIntegerSlider* opaqueMax = (SoDialogIntegerSlider*)myTopLevelDialog->searchForAuditorId(SbString("opaque max"));
  opaqueMax->max.setValue(255);
  opaqueMax->min.setValue(0);
  opaqueMax->value.setValue(m_maxOpaqueMap);
  SoDialogIntegerSlider* opaqueMin = (SoDialogIntegerSlider*)myTopLevelDialog->searchForAuditorId(SbString("opaque min"));
  opaqueMin->max.setValue(255);
  opaqueMin->min.setValue(0);
  opaqueMin->value.setValue(m_minOpaqueMap);

  SoDialogIntegerSlider* sliceNumber = (SoDialogIntegerSlider*)myTopLevelDialog->searchForAuditorId(SbString("slicenumber"));

  switch (m_sliceOri) {     //mmh
  case 0: // X
    sliceNumber->max.setValue(m_width-1);
    sliceNumber->value.setValue(1);
    break;
  case 1: // Y
    sliceNumber->max.setValue(m_height-1);
    sliceNumber->value.setValue(1);
    break;
  case 2: // Z
    sliceNumber->max.setValue(m_depth-1);
    sliceNumber->value.setValue(1);
    break;
  }

  SoDialogCheckBox* draVis = (SoDialogCheckBox*)myTopLevelDialog->searchForAuditorId(SbString("draggervisibility"));
  draVis->state = FALSE;
  m_volROISwitch->whichChild = m_volROIDraggerSwitch->whichChild = SO_SWITCH_NONE;

  SoDialogCheckBox* constrained = (SoDialogCheckBox*)myTopLevelDialog->searchForAuditorId(SbString("constrained"));
  constrained->state = TRUE;
  m_ROIManip->constrained = TRUE;

  SoRowDialog* rowLDM = (SoRowDialog*)myTopLevelDialog->searchForAuditorId(SbString("ldmTab"));
  rowLDM->enable = TRUE;

  //set up right default file name
  SoDialogComboBox* fileName = (SoDialogComboBox*)myTopLevelDialog->searchForAuditorId(SbString("data"));
  fileName->selectedItem = m_defaultFile;

  // Subvolume size/bounds
  m_ROIManip->getDragger()->addValueChangedCallback ( (SoDraggerCB *)&ROIChangedCB, m_ROIManip );
  ROIChangedCB( (void*)m_ROIManip, NULL );

#endif //CAVELIB
}


//jh-dec 2002 for alternate representation
void *writeFile(SoGroup* sep, const SbString& filename)
{
  SoOutput myOutput;
  SoWriteAction myAction(&myOutput);
  if(!myOutput.openFile(filename))
  {
    fprintf(stderr, "cannot open file %s\n", filename.toLatin1());
    return NULL;
  }

  myAction.apply(sep);

  myOutput.closeFile();
  return NULL;
}

/*----------------------------------------------------------------------------------*/
/* Animate orthoslice                                                              */
/*----------------------------------------------------------------------------------*/
static void orthoSliceAnimCB( void *data,  SoSensor * /*sensor*/)
{
  size_t activeSlice = (size_t)data;

  int max;
  switch(m_orthoTab[activeSlice]->axis.getValue()) {
    case SoOrthoSlice::X:
      max = m_width;
      break;
    case SoOrthoSlice::Y:
      max = m_height;
      break;
    case SoOrthoSlice::Z:
      max = m_depth;
      break;
  }

  // If the slice is not animated, just update slider position
  if ( ! m_orthoDataTab[activeSlice].animEnabled )
  {
    if ( activeSlice == (size_t)m_currentSlice )
    {
      SoDialogIntegerSlider* sliceNumber = (SoDialogIntegerSlider*)myTopLevelDialog->searchForAuditorId(SbString("slicenumber"));
      sliceNumber->value = (int32_t)m_orthoTab[activeSlice]->sliceNumber.getValue();
    }
  }
  else
  {
    int currSlice = m_orthoTab[activeSlice]->sliceNumber.getValue();

    //If the user is using the "slice number" slider change the current slide
    if(currSlice != m_orthoDataTab[activeSlice].sliceNumber)
      m_orthoDataTab[activeSlice].sliceNumber = currSlice;

    if(currSlice >= max)
    {
      m_orthoDataTab[activeSlice].dir = -1;
      m_orthoTab[activeSlice]->sliceNumber = (uint32_t)max;
    }
    else if(currSlice <= 0)
    {
      m_orthoDataTab[activeSlice].dir = 1;
      m_orthoTab[activeSlice]->sliceNumber = (uint32_t)0;
    }

    m_orthoDataTab[activeSlice].sliceNumber = (int)(m_orthoDataTab[activeSlice].sliceNumber+m_orthoDataTab[activeSlice].dir);
    m_orthoTab[activeSlice]->sliceNumber = (unsigned int)m_orthoDataTab[activeSlice].sliceNumber;

    if(activeSlice == (size_t)m_currentSlice)
    {
      SoDialogIntegerSlider* sliceNumber = (SoDialogIntegerSlider*)myTopLevelDialog->searchForAuditorId(SbString("slicenumber"));
      sliceNumber->value = (int32_t)m_orthoTab[activeSlice]->sliceNumber.getValue();
    }
  }
}

/*----------------------------------------------------------------------------------
//This function creates mutliple orthoslices
//created : jh-march 2003
//updated : jh-july 2003 add invisible dragger per slice
----------------------------------------------------------------------------------*/
void createOrthoSlices(size_t num, int orthoVis)
{
  if(m_bench.isRecording())
  {
    SoDebugError::post("VolRend","Create orthoSlices before to start recording");
    return;
  }
  if( !m_orthoTab.empty() )
  {
    m_switchForOrthoDragger->removeAllChildren();
    for(int i=0;i<m_lastNumSlices;i++)
    {
      m_groupTab[i]->removeChild( m_orthoTab[i].ptr() );
      m_switchOrthoTab[i]->removeChild( m_groupTab[i].ptr() );
      m_volRenderOrthSliceSwitch->removeChild( m_switchOrthoTab[i].ptr() );
      m_orthoDataTab[i].timerSensor->unschedule();
      delete m_orthoDataTab[i].timerSensor;
    }
    m_groupTab.clear();
    m_orthoQualityTab.clear();
    m_orthoTab.clear();
    m_orthoDataTab.clear();
    m_switchOrthoTab.clear();
  }

  m_groupTab.resize(num);
  m_orthoQualityTab.resize(num);
  m_orthoTab.resize(num);
  m_orthoDataTab.resize(num);
  m_switchOrthoTab.resize(num);
  int space = 0;
  const int defaultFPS = 10;
  for(size_t i=0;i<num;i++)
  {
    //separator
    m_groupTab[i] = new SoGroup;

    //volume rendering quality
    m_orthoQualityTab[i] = new SoVolumeRenderingQuality;
    m_groupTab[i]->addChild( m_orthoQualityTab[i].ptr() );
    m_orthoQualityTab[i]->interpolateOnMove = TRUE;
    m_orthoQualityTab[i]->lightingModel = SoVolumeRenderingQuality::OPENGL;
    m_orthoQualityTab[i]->edgeDetect2DMethod = SoVolumeRenderingQuality::LUMINANCE|SoVolumeRenderingQuality::DEPTH;
    m_orthoQualityTab[i]->surfaceScalarExponent = 8;

    //slice
    m_orthoTab[i] = new SoOrthoSlice;
    m_orthoTab[i]->setName("AOrthoSlice");
    m_orthoTab[i]->axis.setValue(SoOrthoSlice::Z); //Z by default
    m_orthoTab[i]->interpolation.setValue(SoOrthoSlice::LINEAR);
    m_orthoTab[i]->sliceNumber.setValue(space);

    //Structure for orthoslice animation
    m_orthoDataTab[i].animEnabled = false;
    m_orthoDataTab[i].dir = 1;
    m_orthoDataTab[i].timerSensor = NULL;
    m_orthoDataTab[i].sliceNumber = space;
    m_orthoDataTab[i].fps = defaultFPS;

    m_orthoDataTab[i].timerSensor = new SoTimerSensor(orthoSliceAnimCB, (void*)(i));
    m_orthoDataTab[i].timerSensor->setInterval(SbTime(1./(float)defaultFPS));
    m_orthoDataTab[i].timerSensor->setBaseTime(SbTime (0.0));
    m_orthoDataTab[i].timerSensor->schedule();

    //invisible dragger
    SoOrthoSliceDragger *sliceDragger = new SoOrthoSliceDragger;

    //add in sep
    m_groupTab[i]->addChild(m_orthoTab[i].ptr());
    m_switchForOrthoDragger->addChild(sliceDragger);

    //switch for slice
    m_switchOrthoTab[i] = new SoSwitch;
    m_switchOrthoTab[i]->addChild(m_groupTab[i].ptr());
    m_switchOrthoTab[i]->whichChild = orthoVis;// all visible


    //add switch for slice in
    m_volRenderOrthSliceSwitch->addChild(m_switchOrthoTab[i].ptr());

    //init dragger
    m_orthoTab[i]->ref();
    SoSearchAction searchAction;
    searchAction.setSearchingAll(TRUE);
    searchAction.setNode( m_orthoTab[i].ptr());
    searchAction.apply( m_root );
    SoPath *path = searchAction.getPath();
    path->ref();
    sliceDragger->orthoSlicePath.setValue(path);
    sliceDragger->volumeExtent.setValue(m_volData->extent.getValue());
    sliceDragger->volumeDimension.setValue(m_volData->data.getSize());

    path->unref();
    m_orthoTab[i]->unrefNoDelete();

    space += (int)(m_depth/num);
  }

  m_sliceOri = Z;//default
  m_lastNumSlices = (int)num;
  m_currentSlice = 0;

  //reset dragger pos
  int sliceNumber=m_orthoTab[0]->sliceNumber.getValue();
  resetDraggerPos(sliceNumber);

  // Orthoslice mode
  SoDialogCheckBox* visLargeSlice = (SoDialogCheckBox*)myTopLevelDialog->searchForAuditorId(SbString("largeslicesupport"));
  visLargeSlice->state.setValue(FALSE);
}

void resetDraggerPos(int sliceNumber)
{
  float x, y, z, xfrac, yfrac, zfrac;
  xfrac = yfrac = zfrac = 0.5;

  m_draggerNormal.setValue(0,0,1);
  zfrac = (float)sliceNumber / (float)(m_depth - 1);

  x = m_minWidth  + xfrac * (m_maxWidth - m_minWidth);
  y = m_minHeight + yfrac * (m_maxHeight - m_minHeight);
  z = m_minDepth  + zfrac * (m_maxDepth - m_minDepth);
  m_draggerSlicePos.setValue( x, y, z );
  setDraggerSlicePos( m_draggerSlicePos );
  m_draggerVolRender->enableValueChangedCallbacks( FALSE );
  m_draggerVolRender->translation.setValue( x, y, z );
  m_draggerVolRender->enableValueChangedCallbacks( TRUE );
}

void updateOrthoSliceMenu(int sliceID)
{
#ifndef CAVELIB

  SoDialogComboBox* select = (SoDialogComboBox*)myTopLevelDialog->searchForAuditorId(SbString("selectedslice"));
  select->selectedItem = sliceID;

  SoDialogCheckBox* orthovisibility = (SoDialogCheckBox*)myTopLevelDialog->searchForAuditorId(SbString("orthovisibility"));
  orthovisibility->state = (m_switchOrthoTab[sliceID]->whichChild.getValue()==SO_SWITCH_NONE)?0:1;

  SoDialogCheckBox* orthoclipping = (SoDialogCheckBox*)myTopLevelDialog->searchForAuditorId(SbString("orthoclipping"));
  orthoclipping->state = m_orthoTab[sliceID]->clipping.getValue()?1:0;

  SoDialogCheckBox* orthoclippingside = (SoDialogCheckBox*)myTopLevelDialog->searchForAuditorId(SbString("orthoclippingside"));
  orthoclippingside->state = (m_orthoTab[sliceID]->clippingSide.getValue()==SoOrthoSlice::FRONT)?0:1;

  SoDialogComboBox* draggerorientation = (SoDialogComboBox*)myTopLevelDialog->searchForAuditorId(SbString("draggerorientation"));
  draggerorientation->selectedItem = m_orthoTab[sliceID]->axis.getValue();

  SoDialogIntegerSlider* slicenumber = (SoDialogIntegerSlider*)myTopLevelDialog->searchForAuditorId(SbString("slicenumber"));
  int num = m_orthoTab[sliceID]->sliceNumber.getValue();
  slicenumber->value.setValue(num);

  SoDialogComboBox* orthoalphatype = (SoDialogComboBox*)myTopLevelDialog->searchForAuditorId(SbString("orthoalphatype"));
  orthoalphatype->selectedItem = m_orthoTab[sliceID]->alphaUse.getValue();

  SoDialogComboBox* orthointerpolation = (SoDialogComboBox*)myTopLevelDialog->searchForAuditorId(SbString("orthointerpolation"));
  switch ( m_orthoTab[sliceID]->interpolation.getValue() )
  {
  case SoOrthoSlice::NEAREST:
    orthointerpolation->selectedItem = 0;
    break;
  case SoOrthoSlice::LINEAR:
    orthointerpolation->selectedItem = 1;
    break;
  case SoOrthoSlice::MULTISAMPLE_12:
    orthointerpolation->selectedItem = 2;
    break;
  default:
    SoError::post("Unhandled interpolation type");
    break;
  }

  SoDialogCheckBox* orthobump = (SoDialogCheckBox*)myTopLevelDialog->searchForAuditorId(SbString("orthobump"));
  orthobump->state = m_orthoTab[sliceID]->enableBumpMapping.getValue();

  char editBuffer[128];
  SoDialogEditText *pEdit = (SoDialogEditText*)myTopLevelDialog->searchForAuditorId( "bumpscale" );
  sprintf( editBuffer, "%f", m_orthoTab[sliceID]->bumpScale.getValue() );
  pEdit->editText = editBuffer;

  SoDialogPushButton* animbutton = (SoDialogPushButton*)myTopLevelDialog->searchForAuditorId(SbString("animbutton"));
  animbutton->buttonLabel = m_orthoDataTab[sliceID].animEnabled?"Stop animation":"Start animation";

  SoDialogIntegerSlider* speed = (SoDialogIntegerSlider*)myTopLevelDialog->searchForAuditorId(SbString("animspeed"));
  speed->value.setValue(m_orthoDataTab[sliceID].fps);

#endif //CAVELIB
}

void updateSelectSliceMenu(int num)
{
#ifndef CAVELIB

  //update selection menu
  SoDialogComboBox* select = (SoDialogComboBox*)myTopLevelDialog->searchForAuditorId(SbString("selectedslice"));
  select->items.deleteValues(0,10);
  for(int i=0;i<num;i++)
  {
    char buffer [33];
    sprintf( buffer, "%d", i+1 );
    select->items.set1Value(i,buffer);
  }

#endif //CAVELIB
}

void updateSelectIsoMenu(int num)
{
#ifndef CAVELIB
  //update selection menu
  SoDialogComboBox* select = (SoDialogComboBox*)myTopLevelDialog->searchForAuditorId(SbString("selectediso"));
  int selItem = select->selectedItem.getValue();
  select->items.deleteValues(0,6);
  for(int i=0;i<num;i++)
  {
    char buffer [33];
    sprintf( buffer, "%d", i+1 );
    select->items.set1Value(i,buffer);
  }
  select->selectedItem = MIN(selItem, select->items.getNum()-1);
#endif //CAVELIB
}

void updateIsoSlider()
{
#ifndef CAVELIB
  SoDialogRealSlider* isoSlider = (SoDialogRealSlider*)myTopLevelDialog->searchForAuditorId(SbString("isovalue"));
  isoSlider->value = m_volIsosurface->isovalues[m_currentIso];
#endif //CAVELIB
}

void updateSelectMaterialMenu(int num)
{
#ifndef CAVELIB

  //update selection menu
  SoDialogComboBox* select = (SoDialogComboBox*)myTopLevelDialog->searchForAuditorId(SbString("selectedmaterial"));
  int selItem = select->selectedItem.getValue();
  select->items.deleteValues(0,6);
  for(int i=0;i<num;i++)
  {
    char buffer [33];
    sprintf( buffer, "%d", i+1 );
    select->items.set1Value(i,buffer);
  }
  select->selectedItem = MIN(selItem, select->items.getNum()-1);
#endif //CAVELIB
}

void updateIsoInterpolationMethod(int index)
{
#ifndef CAVELIB
  SoDialogRealSlider* slideSegmentedThresholdIso = (SoDialogRealSlider*)myTopLevelDialog->searchForAuditorId(SbString("segmentedthreshold"));
  m_volQuality->colorInterpolation = (index==0 || index == 1);
  m_volQuality->segmentedInterpolation = (index==1);
  slideSegmentedThresholdIso->enable = (index==1);
#endif //CAVELIB
}

void switchInterfaceToIsoMode(bool enable)
{
#ifndef CAVELIB
  SoDialogComboBox* cboxMaterial = (SoDialogComboBox*)myTopLevelDialog->searchForAuditorId(SbString("numberOfMaterial"));
  SoDialogComboBox* cboxSelMaterial = (SoDialogComboBox*)myTopLevelDialog->searchForAuditorId(SbString("selectedmaterial"));
  SoDialogComboBox* cboxIso = (SoDialogComboBox*)myTopLevelDialog->searchForAuditorId(SbString("numberOfIso"));
  SoDialogComboBox* cboxSelIso = (SoDialogComboBox*)myTopLevelDialog->searchForAuditorId(SbString("selectediso"));
  SoDialogRealSlider* islidIso = (SoDialogRealSlider*)myTopLevelDialog->searchForAuditorId(SbString("isovalue"));
  SoDialogComboBox* cboxInterpolationMethodIso = (SoDialogComboBox*)myTopLevelDialog->searchForAuditorId(SbString("interpolationmethod"));
  SoDialogRealSlider* slideSegmentedThresholdIso = (SoDialogRealSlider*)myTopLevelDialog->searchForAuditorId(SbString("segmentedthreshold"));

  cboxSelMaterial->enable = enable;
  cboxMaterial->enable = enable;
  cboxIso->enable = enable;
  cboxSelIso->enable = enable;
  islidIso->enable = enable;
  cboxInterpolationMethodIso->enable = enable;
  slideSegmentedThresholdIso->enable = enable;

  if ( enable )
    updateIsoInterpolationMethod(cboxInterpolationMethodIso->selectedItem.getValue());

  m_mtlEditor->detach();
  if(enable)
    m_mtlEditor->attach(m_material, m_currentMaterial);
  else {
    m_mtlEditor->attach(m_material, 0);
    cboxSelMaterial->selectedItem = 0;
    m_currentMaterial = 0;
  }
#endif //CAVELIB
}

void activeEdgesAndJitter(bool enable)
{
#ifndef CAVELIB
  SoDialogCheckBox* jitter = (SoDialogCheckBox*)myTopLevelDialog->searchForAuditorId(SbString("jittering"));

  SoRowDialog* edgeDiag = (SoRowDialog*)myTopLevelDialog->searchForAuditorId(SbString("edgeDialogId"));

  jitter->enable = enable;
  edgeDiag->enable = enable;
#endif //CAVELIB
}

void activeSurfaceScalar(bool enable)
{
#ifndef CAVELIB
  SoDialogViz* dv = myTopLevelDialog->searchForAuditorId(SbString("normalizegradient"));
  dv->enable = enable;

  dv = myTopLevelDialog->searchForAuditorId(SbString("surfacescalar"));
  dv->enable = enable;
#endif //CAVELIB
}

void switchInterfaceToVolumeSkinMode(bool enable)
{
#ifndef CAVELIB
  SoDialogCheckBox* vsLargeSliceSupport = (SoDialogCheckBox*)myTopLevelDialog->searchForAuditorId(SbString("VSlargeSliceSupport"));
  SoDialogComboBox* vsFaceMode = (SoDialogComboBox*)myTopLevelDialog->searchForAuditorId(SbString("skinfacemode"));
  SoDialogCheckBox* vsBump = (SoDialogCheckBox*)myTopLevelDialog->searchForAuditorId(SbString("skinbump"));
  SoDialogEditText* vsBumpScale = (SoDialogEditText*)myTopLevelDialog->searchForAuditorId(SbString("skinbumpscale"));
  SoDialogCheckBox* vsOutline = (SoDialogCheckBox*)myTopLevelDialog->searchForAuditorId(SbString("skinoutlined"));

  vsLargeSliceSupport->enable = enable;
  vsFaceMode->enable = enable;
  vsBump->enable = enable;
  vsBumpScale->enable = enable;
  vsOutline->enable = enable;
#endif //CAVELIB
}

void fixedResCB(SoVolumeData::LDMResourceParameter::FixedResolutionReport& report,void* /*vData*/)
{
  char buf[256];

  switch(report.what)
  {
  case SoVolumeData::LDMResourceParameter::FixedResolutionReport::PROGRESS:
    {
      if(report.numTilesToLoad){
        float percentage = (float)(report.numTilesLoaded)* 100.f / (float)(report.numTilesToLoad);
        sprintf( buf, "%g percent of data loaded", percentage );
      }
      break;
    }
  case SoVolumeData::LDMResourceParameter::FixedResolutionReport::ABORT:
    {
      if (report.numTilesToAdd){
        sprintf( buf, "Request is not possible : %d tiles must be added to main memory\n", report.numTilesToAdd);
      }
      else
        sprintf( buf, "Request is not possible : the data does not have the specified resolution\n");

      SoDialogCheckBox* enableMenu = (SoDialogCheckBox*)myTopLevelDialog->searchForAuditorId(           SbString("fixedresmode"));
      enableMenu->state = false;

      break;
    }

  default:
    return ;
  }

#if defined(_WIN32) && !defined(SOQT)
  SbString str(buf) ;
  SoConsole* console = SoXt::getErrorConsole();
  SoConsole::setMaxNumMessages(1);
  console->printMessage(str);
#else
  printf("%s\n", buf) ;
#endif
}

#include <Inventor/nodes/SoCamera.h>

///////////////////////////////////////////////////////////////////////
//
// Update UI when ROI subvolume changes

void ROIChangedCB( void *data, SoDragger *dragger)
{
  SoROIManip* pManip = (SoROIManip*)data;
  SbBox3i32   subvol = pManip->subVolume.getValue();

  // Display ROI dimensions in user interface
  int xmin,ymin,zmin,xmax,ymax,zmax;
  subvol.getBounds( xmin, ymin, zmin, xmax, ymax, zmax );
  updateROIdisplay( xmin, ymin, zmin, xmax, ymax, zmax );

  // Move camera if locked to ROI position
  // Note: This function may be called with dragger = NULL !
  SoTabBoxDragger *pDragger = (SoTabBoxDragger *)dragger;
  if (pDragger) {
    if (m_camLockToROI) {
      SbVec3f newCenter = pDragger->translation.getValue();
      SbVec3f diff = newCenter - m_ROIcenter;
      if (diff.length() > 0) {
        SoCamera *pCam = m_viewer->getCamera();
        if (pCam) {
          SbVec3f pos = pCam->position.getValue();
          pCam->position = pos + diff;
        }
      }
    }
    // Remember dragger position for later
    m_ROIcenter = pDragger->translation.getValue();
  }
}

///////////////////////////////////////////////////////////////////////
//
// Update ROI display in UI

void updateROIdisplay( int xmin, int ymin, int zmin, int xmax, int ymax, int zmax )
{
  char editBuffer[128];
  SoDialogEditText *pEdit;

  pEdit = (SoDialogEditText*)myTopLevelDialog->searchForAuditorId( "roi_min_x" );
  sprintf( editBuffer, "%d", xmin );
  pEdit->editText = editBuffer;
  pEdit = (SoDialogEditText*)myTopLevelDialog->searchForAuditorId( "roi_min_y" );
  sprintf( editBuffer, "%d", ymin );
  pEdit->editText = editBuffer;
  pEdit = (SoDialogEditText*)myTopLevelDialog->searchForAuditorId( "roi_min_z" );
  sprintf( editBuffer, "%d", zmin );
  pEdit->editText = editBuffer;

  pEdit = (SoDialogEditText*)myTopLevelDialog->searchForAuditorId( "roi_max_x" );
  sprintf( editBuffer, "%d", xmax );
  pEdit->editText = editBuffer;
  pEdit = (SoDialogEditText*)myTopLevelDialog->searchForAuditorId( "roi_max_y" );
  sprintf( editBuffer, "%d", ymax );
  pEdit->editText = editBuffer;
  pEdit = (SoDialogEditText*)myTopLevelDialog->searchForAuditorId( "roi_max_z" );
  sprintf( editBuffer, "%d", zmax );
  pEdit->editText = editBuffer;

  pEdit = (SoDialogEditText*)myTopLevelDialog->searchForAuditorId( "roi_size_x" );
  sprintf( editBuffer, "%d", xmax-xmin+1 );
  pEdit->editText = editBuffer;
  pEdit = (SoDialogEditText*)myTopLevelDialog->searchForAuditorId( "roi_size_y" );
  sprintf( editBuffer, "%d", ymax-ymin+1 );
  pEdit->editText = editBuffer;
  pEdit = (SoDialogEditText*)myTopLevelDialog->searchForAuditorId( "roi_size_z" );
  sprintf( editBuffer, "%d", zmax-zmin+1 );
  pEdit->editText = editBuffer;
}

/************************************************************************************/
void createGrids(size_t num)
{
  m_predefinedGrid.reserve(num);
  m_userGrid.reserve(num);
  SoChildList *children = m_gridsGroup->getChildren();
  size_t numChildren = children->getLength();

  if(numChildren == num)
    return;

  if(numChildren > num) {
    int numToRemove = (int)(numChildren-num);
    for(int i = 0; i < numToRemove; i++) {
      m_gridsGroup->removeChild((int)(numChildren-1-i));
      m_predefinedGrid[numChildren-i-1] = 0;
      m_userGrid[numChildren-i-1] = SbString("");
    }
    return;
  }

  float sx, sy, sz;
  m_volData->extent.getValue().getSize(sx, sy, sz);

  int numToAdd = (int)(num-numChildren);
  for(int i = 0; i < numToAdd; i++) {
    SoUniformGridClipping *grid = createDefaultGrid();
    float minHeight = m_minHeight + (float)(numChildren + i) * .3f * sy;
    grid->extent = SbBox3f(m_minWidth, minHeight, m_minDepth, m_maxWidth, minHeight+sy/2, m_maxDepth);
    addGridToSceneGraph(grid,  (int)(2+numChildren+i));
    m_predefinedGrid[numChildren+i-1] = 0;
    m_userGrid[numChildren+i-1] = SbString("");
  }
}

void updateSelectGridMenu(int num)
{
  SoDialogComboBox* create = (SoDialogComboBox*)myTopLevelDialog->searchForAuditorId(SbString("numberOfGridsID"));
  //update selection menu
  SoDialogComboBox* select = (SoDialogComboBox*)myTopLevelDialog->searchForAuditorId(SbString("selectedGrid"));
  select->items.deleteValues(0);
  for(int i=0;i<num;i++)
  {
    char buffer [33];
    sprintf( buffer, "%d", i+1 );
    select->items.set1Value(i,buffer);
    create->items.set1Value(i,buffer);
  }
  create->selectedItem = num-1;
}

void addGridToSceneGraph(SoUniformGridClipping *grid, int unit)
{
  SoJackManip *manipGrid = new SoJackManip;
  SoTextureUnit *texUnit = new SoTextureUnit;
  texUnit->unit = unit;

  SoSwitch *gridSwitch = new SoSwitch;
  gridSwitch->whichChild = SO_SWITCH_ALL;
  gridSwitch->setName("GridGroup");
  gridSwitch->addChild(manipGrid);
  gridSwitch->addChild(texUnit);
  gridSwitch->addChild(grid);
  gridSwitch->addChild(new SoResetTransform);
  m_gridsGroup->addChild(gridSwitch);
}

void addClipVolumeToSceneGraph(SoVolumeClippingGroup *cg)
{
  SoTransformerManip *manip = new SoTransformerManip;
  SoSwitch *clipGroupSwitch = new SoSwitch;
  clipGroupSwitch->whichChild = SO_SWITCH_ALL;
  m_volumeClippingSwitch->addChild(clipGroupSwitch);
  clipGroupSwitch->addChild(m_volumeClippingTransform);
  clipGroupSwitch->addChild(manip);

  //Allow to swith model visibility on/off
  SoSeparator *sep = new SoSeparator;
  SoMaterial *mat = new SoMaterial;
  mat->transparency = 0.8f;
  SoTransparencyType *tp = new SoTransparencyType;
  tp->type = SoTransparencyType::SORTED_OBJECT_ADD;
  m_volumeClippingVisibleSwitch->addChild(tp);
  m_volumeClippingVisibleSwitch->addChild(mat);
  m_volumeClippingVisibleSwitch->addChild(cg->getChild(0));
  sep->addChild(m_volumeClippingVisibleSwitch);
  clipGroupSwitch->addChild(sep);

  clipGroupSwitch->addChild(cg);
  clipGroupSwitch->addChild(new SoResetTransform);
}

SoVolumeClippingGroup *getVolumeClippingGroup(int /*n*/)
{
  SoSwitch *volClipSw = (SoSwitch *)m_volumeClippingSwitch->getChild(m_selectedVolume);
  return (SoVolumeClippingGroup *)volClipSw->getChild(3);
}

SoTransform *getVolumeClippingGroupManip(int n)
{
  SoSwitch *volClipSw = (SoSwitch *)m_volumeClippingSwitch->getChild(n);
  return (SoTransform *)volClipSw->getChild(1);
}

void changeVolumeClippingGroupManip(int n, SoTransform *transform)
{
  SoSwitch *volClipSw = (SoSwitch *)m_volumeClippingSwitch->getChild(n);
  volClipSw->replaceChild(1, transform);
}

void
switchOptimInterfaceToExpert(bool flag)
{
  SoDialogCheckBox* cboxGpuVert = (SoDialogCheckBox*)myTopLevelDialog->searchForAuditorId(SbString("gpuvertgen"));
  SoDialogCheckBox* cboxSubDivTile = (SoDialogCheckBox*)myTopLevelDialog->searchForAuditorId(SbString("subdivideTile"));
  SoDialogCheckBox* cboxEarlyz = (SoDialogCheckBox*)myTopLevelDialog->searchForAuditorId(SbString("earlyz"));
  SoDialogIntegerSlider* slideNumPass = (SoDialogIntegerSlider*)myTopLevelDialog->searchForAuditorId(SbString("zpass"));


  cboxGpuVert->enable = flag;
  cboxSubDivTile->enable = flag;
  cboxEarlyz->enable = flag;
  slideNumPass->enable = flag;

  if ( flag )
  {
    m_volRend->gpuVertexGen =  cboxGpuVert->state.getValue();
    m_volRend->subdivideTile = cboxSubDivTile->state.getValue();
    m_volRend->useEarlyZ = cboxEarlyz->state.getValue();
    m_volRend->numEarlyZPasses = slideNumPass->value.getValue();
  }
}


