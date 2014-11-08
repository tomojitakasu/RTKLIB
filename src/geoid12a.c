/*
| Description:
|
|   United States National Geodetic Survey (NGS) GEOID12A geoid model
|   (and several other related NGS geoid models) for RTKLIB.
|
|   Written in August 2014 by Daniel A. Cook
|   Spline core function by Rene Forsberg.
|
|   Provides RTKLIB library level support for the following NGS geoids:
|
|	G99SSS. GEOID99, GEOID03, GEOID06, GEOID09, GEOID12A,
|	USGG2003, USGG2009, USGG2012, XXUSG.
|
|   Coverage: CONUS, Alaska, Hawaii, PR/VI, Guam/NMI, American Samoa.
|
|   Interpolates the geoid height for a user specified position and geoid model.
|   Estimates within gridded data models using spline or bilinear interpolation. 
|
|   Uses NGS gridded BINARY format data model files (*.bin) freely available for
|   download from the NGS website:
|
|	<http://www.ngs.noaa.gov/GEOID/models.shtml>
|
|   NGS defines these geoids as well as the filenames of the geoid data files.
|   Just download the BINARY files for your selected geoid from the NGS website
|   and copy them to the directory of choice on your computer. Specify this
|   directory in RTKLIB where you would normally specify a geoid data file.
|   Only the directory (and drive letter on the Windows platform) portion of
|   the full file path will be used. Data files for several geoids can be
|   placed into the same directory. Each geoid may have one or several (perhaps
|   as many as 16) data files associated with it. Do not rename the NGS
|   provided data files. The names are important and must remain as named by
|   NGS including case on case sensitive filesystems.
|
|   Some of these geoids are datum and epoch specific. No datum or epoch
|   transformations are performed herein. It is up to the user to be using
|   the correct datum and epoch, such as by choosing a base such that RTKLIB
|   ends up working in the datum of the base.
|
|   Both big and little endian geoid data file content can be mixed without
|   any need to match the endianness of the execution platform. Endian
|   detection and conversion is handled automatically. Therefore you should
|   be able to download and use any mix of big and/or little endian format
|   data files without regard to the platform on which RTKLIB is run.
|
| Design Issues:
|
|   The order of bytes in the geoid model data files are dependent on which
|   platform the file was created. The header data in each file contains a
|   thoughtful "Endian" header field which allows us to automatically detect
|   and correct for endian differences.
|
| References:
|
|   1. The NGS Geoid Page
|      <http://www.ngs.noaa.gov/GEOID/
|
|   2. NGS GEOID12A Home Page
|      <http://www.ngs.noaa.gov/GEOID/GEOID12A/>
|
|   3. NGS Geoid and Deflection Models
|      <http://www.ngs.noaa.gov/GEOID/models.shtml>
|
|   4. RTKLIB Version 2.4.2 Manual, April 29 2013
|      <http://www.rtklib.com/prog/manual_2.4.2.pdf>
*/
			

/*
| Included files:
*/

#include "rtklib.h"
#ifndef WIN32
#include <libgen.h>
#endif

/*
| Constant definitions:
*/
#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE  !FALSE
#endif

#define MAXGRIDFILES 32

/*
| Internal structure definitions.
*/
typedef struct       /* Geoid grid data file header */
{   double LatMin;   /* Real*8, North Latitude  (degrees) */
    double LonMin;   /* Real*8, East  Longitude (degrees) */
    double LatDelta; /* Real*8, (degrees) */
    double LonDelta; /* Real*8, (degrees) */
    int    LatNum;   /* I4 */
    int    LonNum;   /* I4 */
    int    Endian;   /* I4 */
} HEADER;


/*
| Static global literals:
*/
static const char rcsid[]="$Id:$";


/*
| Static global variables:
*/
static int    CurrentGeoid = 0;
static int    NumberOfGeoidFiles = 0;
static FILE*  FilePointerArray[MAXGRIDFILES];
static char   FileNameArray[MAXGRIDFILES][MAXSTRPATH];
static HEADER FileHeaderArray[MAXGRIDFILES];

/*
| Internal private function forward declarations (in alphabetical order):
*/
static double BiLinear(double Latitude, double Longitude, int GeoidFile);
static void   GeoidFileName(char *Directory, char *Name, char *FileName);
static void   GeoidFileNames(int First, int Last, int Offset, char *Directory, char *Prefix, char *Sequence);
static char   *GeoidName(int Geoid);
static void   GetDirectory(char *Path, char *Directory);
static void   InitializeSpline(double *YY, int WindowSize, double *RR, double *QQ);
static double InterpolateGeoidHeight(double Latitude, double Longitude, int GeoidFile);
static int    OpenGeoidFiles(int Geoid, char *Directory);
static int    ReadGeoidFileHeaders(void);
static int    SelectBestGeoidFile(int *BestGeoidFile, double Latitude, double Longitude);
static double Spline(double X, double *YY, int WindowSize, double *RR);
static double Spline4(double Latitude, double Longitude, int GeoidFile);
static double Spline6(double Latitude, double Longitude, int GeoidFile);
static void   Swap4(void *p);
static void   Swap8(void *p);

/*
| Public functions (in alphabetical order):
*/


/*
| Function: closegeoid12a
| Purpose:  Close geoid grid files, cleanup, re-initialize
| Authors:  Daniel A. Cook
|
| Formal Parameters: 
|
|   <none>
|
| Implicit Inputs:
|
|   NumberOfGeoidFiles
|   FilePointerArray
|
| Implicit Outputs:
|
|   CurrentGeoid
|   NumberOfGeoidFiles
|   FilePointerArray
|   FileNameArray
|   FileHeaderArray
|
| Return Value:
|
|   <none>
|
| Design Issues:
|
*/
extern void closegeoid12a(void)
{
    int n;

    if (NumberOfGeoidFiles)
    {
	for (n = 0; n < NumberOfGeoidFiles; n++)
	{
	    if (FilePointerArray[n])
		fclose(FilePointerArray[n]);
	}
    }

    memset(FilePointerArray, 0, sizeof(FilePointerArray));
    memset(FileNameArray, 0, sizeof(FileNameArray));
    memset(FileHeaderArray, 0, sizeof(FileHeaderArray));

    NumberOfGeoidFiles = 0;
    CurrentGeoid = 0;
}

/*
| Function: geoidh_geoid12a
| Purpose:  Transform elipsoid height to GEOID12A orthometric height.
| Authors:  Daniel A. Cook
|
| Formal Parameters: 
|
|   Latitude  = Latitude (degrees)  [Input]
|   Longitude = Longitude (degrees) [Input]
|
| Implicit Inputs:
|
|   CurrentGeoid
|
| Implicit Outputs:
|
|   <none>
|
| Return Value:
|
|   Geoid Height
|
| Design Issues:
|
*/
extern double geoidh_geoid12a(double Latitude, double Longitude)
{
    int BestGeoidFile;
  
    if (Longitude < 0)
	Longitude += 360;

    /*
    | Select the best geoid grid file to use, based on the latitude and longitude.
    | Interpolate the geoid height.
    */
    if (SelectBestGeoidFile(&BestGeoidFile, Latitude, Longitude))
	return (InterpolateGeoidHeight(Latitude, Longitude, BestGeoidFile));

    trace( 2, "GEOID12A: failed to determine %s geoid height.\n",
	   GeoidName(CurrentGeoid) );
    return (0.0);
}

/*
| Function: opengeoid12a
| Purpose:  Open geoid grid files
| Authors:  Daniel A. Cook
|
| Formal Parameters: 
|
|   Geoid     = Geoid to use              [Input]
|   Directory = Geoid grid file directory [Input]
|
| Implicit Inputs:
|
|   <none>
|
| Implicit Outputs:
|
|   <none>
|
| Return Value:
|
|   TRUE  = Success
|   FALSE = Failure
|
| Design Issues:
|
*/
extern int opengeoid12a(int Geoid, const char *Directory)
{
    /*
    | Clean up and re-initialize.
    */
    closegeoid12a();

    /*
    | Create the list of grid files that must be opened and open them.
    |
    | Read the headers of all geoid grid files that were opened and store
    | that information. Compute the maximum latitude and longitude from the
    | header information. Apply endian correction as needed.
    */
    if (!(OpenGeoidFiles(Geoid, (char*) Directory) && ReadGeoidFileHeaders()))
    {
        closegeoid12a();
	return (FALSE);
    }

    return (TRUE);
}


/*
| Private functions (in alphabetical order):
*/


/*
| Function: BiLinear
| Purpose:  Bilinear Interpolation
| Authors:  Daniel A. Cook
|
| Formal Parameters: 
|
|   Latitude  = North latitude  of point [Input]
|   Longitude = East  longitude of point [Input]
|   GeoidFile = Grid file number         [Input]

| Implicit Inputs:
|
|   FilePointerArray
|   FileHeaderArray 
|   Geoid data file content
|
| Implicit Outputs:
|
|   <none>
|
| Return Value:
|   
|   Interpolated geoid height.
|
| Design Issues:
|
*/
static double BiLinear(double Latitude, double Longitude, int GeoidFile)
{
    double   LatMin, LatMax, LatDelta, LonMin, LonDelta;
    double   ColCounter, RowCounter, onemxx, onemyy;
    float    UL, UR, LL, LR, xx, yy;
    long     LonColNum, LatRowNum, IColN, IRowN, IRec;
    int      Endian;  
    FILE*    FilePointer;

    /*
    | Define some necessary parameters.
    */
    FilePointer = FilePointerArray[GeoidFile];
    LatMin      = FileHeaderArray[GeoidFile].LatMin;
    LonMin      = FileHeaderArray[GeoidFile].LonMin;
    LatDelta    = FileHeaderArray[GeoidFile].LatDelta;
    LonDelta    = FileHeaderArray[GeoidFile].LonDelta;
    LatRowNum   = FileHeaderArray[GeoidFile].LatNum;
    LonColNum   = FileHeaderArray[GeoidFile].LonNum;
    Endian      = FileHeaderArray[GeoidFile].Endian;
    LatMax      = LatMin + LatDelta * (LatRowNum - 1);
  
    /*
    | Find the row / column of the nearest grid node to the latitude / longitude point.
    | This grid node is Southwest from the latitude / longitude point.
    | (Rowcounter, ColCounter) = exact (latitude, longitude) grid coord location (real*8)
    | (Row, Column)            = closest node (to sw) grid coord loc'n (int)
    */
    RowCounter = ((Latitude  - LatMin) / LatDelta);
    ColCounter = ((Longitude - LonMin) / LonDelta);

    /*
    | Find the reference node row and column.
    */
    IRowN = (long) floor(RowCounter);       /* the row just South from Latitude  */
    IColN = (long) floor(ColCounter);       /* the col just West  from Longitude */

    /*
    | Find relative location of the point in the interpolation window.
    */
    yy = (RowCounter - IRowN);              /* 1x1 window */
    xx = (ColCounter - IColN);

    /*
    | Find relative location of the point in the interpolation window.
    */
    onemyy = 1.0 - yy;
    onemxx = 1.0 - xx;

    /*
    | Extract 2x2 data array around the point and do bilinear interpolation
    |
    |             (IColN)   ----> East   (IColN + 1)
    |
    | (IRowN - 1)  LL ------------------------  LR
    |                 |     xx        | 1mxx |
    |      |          | yy            |      |
    |      |          |               |      |
    |      |          |               |      |
    |      |          |               |      |
    |      v          |----------------      |
    |    North        | 1myy                 |
    |                 |                      |
    |   (IRowN)    UL ------------------------  UR
    */

    /*
    | If point LL in on Northern edge, then UL and UR are outside of dataset
    | Linear interpolation on or near Northern edge only (1*e-4 = 0.3 arcsec)
    */
    if ((LatMax - Latitude) < 0.0001)
    {
        IRec = 44L + (long) (4 * (IRowN * LonColNum + IColN));
        fseek(FilePointer, IRec, SEEK_SET);
        fread((char*) &LL, sizeof(float), 1, FilePointer);

        IRec = 44L + (long) (4 * (IRowN * LonColNum + IColN + 1));
        fseek(FilePointer, IRec, SEEK_SET);
        fread((char*) &LR, sizeof(float), 1, FilePointer);

	/*
	| Swap bytes as needed.
	*/
        if (Endian != 1)
	{
            Swap4(&LL);
            Swap4(&LR);
        }

        return ((LL * onemxx / (xx + onemxx)) + (LR * xx / (xx + onemxx)));
    }

    /*
    | Bilinear interpolation.
    */
    IRec = 44L + (long) (4 * (IRowN * LonColNum + IColN));
    fseek(FilePointer, IRec, SEEK_SET);
    fread((char*) &LL, sizeof(float), 1, FilePointer);

    IRec = 44L + (long) (4 * (IRowN * LonColNum + (IColN + 1)));
    fseek(FilePointer, IRec, SEEK_SET);
    fread((char*) &LR, sizeof(float), 1, FilePointer);

    IRec = 44L + (long) (4 * ((IRowN+1) * LonColNum + IColN));
    fseek(FilePointer, IRec, SEEK_SET);
    fread((char*) &UL, sizeof(float), 1, FilePointer);

    IRec = 44L + (long) (4 * ((IRowN+1) * LonColNum + (IColN+1)));
    fseek(FilePointer, IRec, SEEK_SET);
    fread((char*) &UR, sizeof(float), 1, FilePointer);

    /*
    | Swap bytes as needed.
    */
    if (Endian != 1)
    {
        Swap4(&LL);
        Swap4(&LR);
        Swap4(&UL);
        Swap4(&UR);
    }

    return (LL * onemyy * onemxx + LR * xx * onemyy + UL * onemxx * yy + UR * xx * yy);
}


/*
| Function: GeoidName
| Purpose:  Convert geoid number to pointer to geoid name string
| Authors:  Daniel A. Cook
|
| Formal Parameters: 
|
|   Geoid = Geoid number [Input]
|
| Implicit Inputs:
|
|   <none>
|
| Implicit Outputs:
|
|   <none>
|
| Return Value:
|
|   Pointer to geoid name string.
|
| Design Issues:
|
|   If you're wondering why this function is overly complicated, and uses
|   a switch statement instead of simply indexing directly into a static
|   const character string array by geoid number, it's because I need to
|   allow T.TAKASU and others the freedom to redefine RTKLIB's GEOID_*
|   #defines in RTKLIB.H without worrying about breaking my code here.
|   If I used the simple index into an array approach and they ever
|   changed the values of the #defines, I'd be screwed.
*/
static char *GeoidName(int Geoid)
{
    static char *g99sss = "G99SSS";
    static char *geoid99 = "GEOID99";
    static char *geoid03 = "GEOID03";
    static char *geoid06 = "GEOID06";
    static char *geoid09 = "GEOID09";
    static char *geoid12a = "GEOID12A";
    static char *usgg2003 = "USGG2003";
    static char *usgg2009 = "USGG2009";
    static char *usgg2012 = "USGG2012";
    static char *xxusg = "XXUSG";
    static char *unsupported = "UNSUPPORTED";

    switch (Geoid)
    {
    case GEOID_G99SSS:
	return (g99sss);
    case GEOID_GEOID99:
	return (geoid99);
    case GEOID_GEOID03:
	return (geoid03);
    case GEOID_GEOID06:
	return (geoid06);
    case GEOID_GEOID09:
	return (geoid09);
    case GEOID_GEOID12A:
	return (geoid12a);
    case GEOID_USGG2003:
	return (usgg2003);
    case GEOID_USGG2009:
	return (usgg2009);
    case GEOID_USGG2012:
	return (usgg2012);
    case GEOID_XXUSG:
	return (xxusg);
    }

    return (unsupported);
}

/*
| Function: GeoidFileName
| Purpose:  Build a geoid grid file name.
| Authors:  Daniel A. Cook
|
| Formal Parameters: 
|
|   Directory = Geoid grid file directory [Input]
|   Name      = File name                 [Input]
|   Filename  = Output file name          [Output]
|
| Implicit Inputs:
|
|   <none>
|
| Implicit Outputs:
|
|   <none>
|
| Return Value:
|
|   <none>
|
| Design Issues:
|
*/
static void GeoidFileName(char *Directory, char *Name, char *Filename)
{
    char TempFilename[MAXSTRPATH] = "";
    if (strlen(Directory))
	strcpy(TempFilename, Directory);
    strcat(TempFilename, Name);
    strcat(TempFilename, ".bin");
    strcpy(Filename, TempFilename);
}

/*
| Function: GeoidFileNames
| Purpose:  Build a sequence of geoid grid file names.
| Authors:  Daniel A. Cook
|
| Formal Parameters: 
|
|   First     = First sequence number     [Input]
|   Last      = Last sequence number      [Input]
|   Offset    = Output file number offset [Input]
|   Directory = Geoid grid file directory [Input]
|   Prefix    = File name prefix          [Input]
|   Format    = Format specifier          [Input]
|
| Implicit Inputs:
|
|   <none>
|
| Implicit Outputs:
|
|   FileNameArray
|
| Return Value:
|
|   <none>
|
| Design Issues:
|
*/
static void GeoidFileNames( int  First,
			    int  Last,
			    int  o,
			    char *Directory,
			    char *Prefix,
			    char *Sequence )
{
    char TempFilename[MAXSTRPATH] = "";
    int n;

    for (n = First; n <= Last; n++)
    {
	if (strlen(Directory))
	    strcpy(TempFilename, Directory);
	strcat(TempFilename, Prefix);
	if (strlen(Sequence))
	{
	    char SN[3] = "";
	    sprintf(SN, Sequence, n - (First - 1));
	    strcat(TempFilename, SN);
	}
        strcat(TempFilename, ".bin");
	strcpy(FileNameArray[o+n-1], TempFilename);
    }
}

/*
| Function: GetDirectory
| Purpose:  Get directory portion of file specification
| Authors:  Daniel A. Cook
|
| Formal Parameters: 
|
|    Path      = Full file specification    [Input]
|    Directory = Only the directory portion [Output]
|
| Implicit Inputs:
|
|   <none>
|
| Implicit Outputs:
|
|   <none>
|
| Return Value:
|
|   <none>
|
| Design Issues:
|
*/
static void GetDirectory(char *Path, char *Directory)
{
#if WIN32
    char Drive[_MAX_DRIVE+_MAX_DIR];
    char Dir[_MAX_DRIVE+_MAX_DIR];
    _splitpath(Path, Drive, Dir, NULL, NULL);
    strcpy(Directory, Drive);
    strcat(Directory, Dir);
#else
    strcpy(Directory, dirname(Path));
    strcat(Directory, "/");
#endif
}

/*
| Function: InitializeSpline
| Purpose:  Fast one-dimensional equidistant spline interpolation initialization
| Authors:  Rene Forsberg
|
| Formal Parameters: 
|
|   YY         = Data values                                     [Input]
|   WindowSize = Spline window size                              [Input]
|   RR         = Spline moments calculated by InitializeSpline() [Output]
|   QQ         = Work Array                                      [Input/OutPut]
|
| Implicit Inputs:
|
|   <none>
|
| Implicit Outputs:
|
|   <none>
|
| Return Value:
|
|   <none>
|
| References:
|
|   JOSEF STOER: EINFUHRUNG IN DIE NUMERISCHE MATHEMATIK, I
|   SPRINGER 1972, PAGE 81.
*/
static void InitializeSpline(double *YY, int WindowSize, double *RR, double *QQ)
{
    double pp;
    int n;

    QQ[0] = 0.0;
    RR[0] = 0.0;

    for (n = 1; n < WindowSize - 1; n++)
    {
        pp     = QQ[n-1] / 2 + 2;
        QQ[n] = -0.5 / pp;
        RR[n] = (3.0 * (YY[n+1] - 2.0 * YY[n] + YY[n-1]) - RR[n-1] / 2.0) / pp;
    }

    RR[WindowSize-1] = 0.0;
    for (n = WindowSize-2; n > 0; n--)
	RR[n] = QQ[n] * RR[n+1] + RR[n];
}

/*
| Function: InterpolateGeoidHeight
| Purpose:  Interpolates the geoid height from a geoid grid file.
| Authors:  Daniel A. Cook
|
| Formal Parameters: 
|
|   Latitude  = Latitude  (degrees) [Input]
|   Longitude = Longitude (degrees) [Input]
|   GeoidFile = Grid file number    [Input]
|
| Implicit Inputs:
|
|   FilePointerArray
|   FileHeaderArray
|   Geoid data file content.
|
| Implicit outputs:
|
|   <none>
|
| Return Value:
|
|   Geoid Height
|
| Design Issues"
|
|   As the gridded data are all direct-access files, only the nearest points
|   (1 thru 9, depending on the point's location relative to corners and edges)
|   are read into RAM for each point's interpolation.
|
|   It is assumed that latitude / longitude fall in the region of the Nth grid.
*/
static double InterpolateGeoidHeight(double Latitude, double Longitude, int GeoidFile)
{
    double LatMin, LatDelta, LonMin, LonDelta;
    double RowCounter, ColCounter;
    double IRowNLat, IColNLon, GeoidHeight = 0.0;
    long   IRec, LatRowNum, LonColNum, IRowN, IColN;
    int    Endian;
    float  f;
    FILE   *FilePointer;

    /*
    | Define some necessary parameters.
    | These header data elements are read in the main driver file,
    | and are already checked for endian condition.
    */
    FilePointer = FilePointerArray[GeoidFile];
    LatMin      = FileHeaderArray[GeoidFile].LatMin;
    LonMin      = FileHeaderArray[GeoidFile].LonMin;
    LatDelta    = FileHeaderArray[GeoidFile].LatDelta;
    LonDelta    = FileHeaderArray[GeoidFile].LonDelta;
    LatRowNum   = FileHeaderArray[GeoidFile].LatNum;
    LonColNum   = FileHeaderArray[GeoidFile].LonNum;
    Endian      = FileHeaderArray[GeoidFile].Endian;
 
    /*
    | A little sanity check for safety's sake.
    | Verify point is within grid file bounds.
    */
    if (Latitude < LatMin)
    {
	trace( 2, "GEOID12A: Latitude (%d) below mininum bound (%d).\n",
	       Latitude, LatMin );
	return (0.0);
    }

    if (Latitude > (LatMin + LatDelta * LatRowNum))
    {
	trace( 2, "GEOID12A: Latitude (%d) above maximum bound (%d).\n",
		Latitude, (int) (LatMin + LatDelta * LatRowNum) );
	return (0.0);
    }

    if (Longitude < LonMin)
    {
	trace( 2, "GEOID12A: Longitude (%d) below minimum bound (%d).\n",
	       Longitude, LonMin );
	return (0.0);
    }

    if (Longitude > (LonMin + LonDelta * LonColNum))
    {
	trace( 2, "GEOID12A: Longitude (%d) above maximum bound (%d).\n",
	       Longitude, LonMin );
	return (0.0);
    }

    /*
    | Find the row/col of the nearest grid node to the lat/lon point
    | This grid node is southwest from the lat/lon point
    | (row_counter,col_counter) = exact (lat,lon) grid coord location (float)
    | (irown,icoln)             = reference node (to sw) grid coord loc'n (int)
    */
    RowCounter = ((Latitude  - LatMin)  / LatDelta);
    ColCounter = ((Longitude - LonMin)  / LonDelta);

    IRowN = (long) floor(RowCounter); /* reference row, just South from latdd */
    IColN = (long) floor(ColCounter); /* reference col, just West  from londd */

    /*
    | Find the latitude and longitude of the nearest grid point.
    */
    IRowNLat = LatMin + LatDelta * IRowN; /* Latitude  just South (up)  from latdd */
    IColNLon = LonMin + LonDelta * IColN; /* Longitude just West (left) from londd */

    /*
    | Find the latitude and longitude of the reference node
    | not needed to find value at a node.
    |
    | If we're sitting right on or near (0.36 arcsec)
    | a grid node, just assign the value and return
    | (1.0e-4 * 3600) = 0.36 arcsec
    */ 
    if ((fabs(Latitude  - IRowNLat) <= 1.0e-4) &&
	(fabs(Longitude - IColNLon) <= 1.0e-4))
    {
	/*
	| Linear array matrix math: 
	| 44L               gets past the header
	| IRowN*LonColNum   gets to the row
	| IColN             gets the data from the specific column
	*/
	IRec = 44L + (long) (4 * (IRowN * LonColNum + IColN));
	fseek(FilePointer, IRec, SEEK_SET);
	fread((char*) &f, (sizeof(float)), 1, FilePointer);

	/*
	| Swap bytes as needed.
	*/
	if (Endian != 1)
	    Swap4(&f);

	return ((double) f);
    }

    /*
    | Not on a node, so interpolate:
    |
    |   1) 6x6 spline grid
    |   2) 4x4 spline grid
    |   3) bilinear
    */
    if ((IRowN >= 3) && (IRowN < (LatRowNum - 2)) &&
	(IColN >= 3) && (IColN < (LonColNum - 2)))
    {
	GeoidHeight = Spline6(Latitude, Longitude, GeoidFile );
    }
    else if ((IRowN >= 2) && (IRowN < (LatRowNum - 1)) &&
	     (IColN >= 2) && (IColN < (LonColNum - 1)))
    {
	GeoidHeight = Spline4(Latitude, Longitude, GeoidFile);
    }
    else
    {
	GeoidHeight = BiLinear(Latitude, Longitude, GeoidFile);
    }

    return (GeoidHeight);
}

/*
| Function: OpenGeoidFiles
| Purpose:  Assemble and open the geoid grid files for the specified geoid.
| Authors:  Daniel A. Cook
|
| Formal Parameters: 
|
|   Geoid = Geoid to use              [Input]
|   Path  = Geoid grid file directory [Input]
|
| Implicit Inputs:
|
|   NumberOfGeoidFiles
|   FilePointerArray
|   FileNameArray
|
| Implicit Outputs:
|
|   NumberOfGeoidFiles
|   FilePointerArray
|   FileNameArray
|
| Return Value:
|
|   TRUE  = Success
|   FALSE = Fail
|
| Design Issues:
|
|   Null file pointer array entries are left in the file pointer array when
|   the corresponding file fails to be successfully opened. These should be
|   skipped over when scanning through the array.
*/
static int OpenGeoidFiles(int Geoid, char *Path)
{
    char FileName[MAXSTRPATH];
    int  FilesOpened = 0;
    int  n, Offset = 0;
    FILE *FilePointer = NULL;
    char Directory[MAXSTRPATH];

    /*
    | We want only the directory portion of the path.
    */
    GetDirectory(Path, Directory);

    switch (Geoid)
    {
    case GEOID_G99SSS:
	/*
	| First 8 files are CONUS.
	*/
	NumberOfGeoidFiles = 8;
	GeoidFileNames(1, 8, 0, Directory, "s1999u", "%02d");
	break;

    case GEOID_GEOID03:
        /*
 	| First 8 files are CONUS.
 	| Next 4 files are ALASKA.
	| Next 1 file is HAWAII.
 	| Next 1 file is PR/VI.
  	*/
	NumberOfGeoidFiles = 14;
	GeoidFileNames(1,   8, 0, Directory, "g2003u", "%02d");
	GeoidFileNames(9,  12, 0, Directory, "g2003a", "%02d");
	GeoidFileNames(13, 13, 0, Directory, "g2003h", "%02d");
	GeoidFileNames(14, 14, 0, Directory, "g2003p", "%02d");  
	break;

    case GEOID_GEOID06:
	/*
	| First n files are CONUS (does not yet exist).
	| First 4 files are ALASKA.
  	*/
	NumberOfGeoidFiles = 4;
	GeoidFileNames(1, 4, 0, Directory, "g2006a", "%02d");
	break;
      
   case GEOID_GEOID09:
	NumberOfGeoidFiles = 16;
    
        /*
	| CONUS - Attempt to open the one file model.
 	*/
	GeoidFileName(Directory, "GEOID09_conus", FileName);
	if ((FilePointer = fopen(FileName, "rb")))
	{
	    fclose(FilePointer);
	    FilePointer = NULL;
	    NumberOfGeoidFiles -= 7;
	    strcpy(FileNameArray[Offset++], FileName);
	}
	else
	{
            /*
	    | First 8 files are CONUS
	    */
	    GeoidFileNames(1, 8, Offset, Directory, "g2009u", "%02d");
	    Offset += 8;
        }

        /*
	| Alaska - Attempt to open the one file model.
	*/
	GeoidFileName(Directory, "GEOID09_ak", FileName);
        if ((FilePointer = fopen(FileName, "rb")))
	{
	    fclose(FilePointer);
	    FilePointer = NULL;
            NumberOfGeoidFiles -= 3;
	    strcpy(FileNameArray[Offset++], FileName);
        }
	else
	{
	    /*
	    | Next 4 files are ALASKA.
    	    */
	    GeoidFileNames(1, 4, Offset, Directory, "g2009a", "%02d");
	    Offset += 4;
	}

	/*
	| Next 1 file is HAWAII.
	| Next 1 file is GUAM - NORTHERN MARIANAS.
 	| Next 1 file is SAMOA.
 	| Next 1 file is PR/VI.
	*/
 	GeoidFileNames(1, 1, Offset++, Directory, "g2009h", "%02d");
	GeoidFileNames(1, 1, Offset++, Directory, "g2009g", "%02d");
	GeoidFileNames(1, 1, Offset++, Directory, "g2009s", "%02d");
	GeoidFileNames(1, 1, Offset,   Directory, "g2009p", "%02d");         
	break;

    case GEOID_GEOID99:
	/*
	| First 8 files are CONUS.
	| Next 4 files are ALASKA.
	| Next 1 file is HAWAII.
	| Next 1 file is PR/VI.
	*/
	NumberOfGeoidFiles = 14;
        GeoidFileNames(1,   8, 0, Directory, "g1999u", "%02d");
	GeoidFileNames(9,  12, 0, Directory, "g1999a", "%02d"); 
	GeoidFileNames(13, 13, 0, Directory, "g1999h", "%02d");
	GeoidFileNames(14, 14, 0, Directory, "g1999p", "%02d");
	break;

   case GEOID_GEOID12A:
        NumberOfGeoidFiles = 16;

	/*
	| CONUS - Attempt to open the one file model.
	*/
	GeoidFileName(Directory, "g2012au0", FileName); 
        if ((FilePointer = fopen(FileName, "rb")))
	{
	    fclose(FilePointer);
	    FilePointer = NULL;
	    NumberOfGeoidFiles -= 7;
	    strcpy(FileNameArray[Offset++], FileName);
	}
	else
	{
	    /*
	    | First 8 files are CONUS.
	    */
	    GeoidFileNames(1, 8, Offset, Directory, "g2012au", "%01d");
	    Offset += 8;
	}

	/*
	| Alaska - Attempt to open the one file model.
	*/
	GeoidFileName(Directory, "g2012aa0", FileName);
        if ((FilePointer = fopen(FileName, "rb")))
	{
	    fclose(FilePointer);
	    FilePointer = NULL;
            NumberOfGeoidFiles -= 3;
            strcpy(FileNameArray[Offset++], FileName);
        }
	else
	{
	    /*
	    | Next 4 files are ALASKA.
	    */
	    GeoidFileNames(1, 4, Offset, Directory, "g2012aa", "%01d");
	    Offset += 4;       
        }

        /*
	| Next 1 file is HAWAII.
	| Next 1 file is GUAM - NORTHERN MARIANAS.
	| Next 1 file is SAMOA.
  	| Next 1 file is PR/VI.
   	*/
	GeoidFileNames(1, 1, Offset++, Directory, "g2012ah0", "%01d");
	GeoidFileNames(1, 1, Offset++, Directory, "g2012ag0", "%01d");
    	GeoidFileNames(1, 1, Offset++, Directory, "g2012as0", "%01d");
	GeoidFileNames(1, 1, Offset,   Directory, "g2012ap0", "%01d");
	break;

   case GEOID_USGG2003:
	/*
	| First 8 files are CONUS.
	| Next 4 files are ALASKA.
 	| Next 1 file is HAWAII.
        | Next 1 file is PR/VI. 
  	*/
	NumberOfGeoidFiles = 14;
	GeoidFileNames(1,   8, 0, Directory, "s2003u", "%02d");
	GeoidFileNames(9,  12, 0, Directory, "s2003a", "%02d");
	GeoidFileNames(13, 13, 0, Directory, "s2003h", "%02d");
 	GeoidFileNames(14, 14, 0, Directory, "s2003p", "%02d");
 	break;
 
    case GEOID_USGG2009:
	/*
	| First 8 files are CONUS.
	| Next 4 files are ALASKA.
	| Next 1 file is HAWAII.
	| Next 1 file is GUAM.
  	| Next 1 file is SAMOA.
  	| Next 1 file is PR/VI.
        */
	NumberOfGeoidFiles = 16;
	GeoidFileNames(1,   8, 0, Directory, "s2009u", "%02d");
	GeoidFileNames(9,  12, 0, Directory, "s2009a", "%02d");
	GeoidFileNames(13, 13, 0, Directory, "s2009h", "%02d");
  	GeoidFileNames(14, 14, 0, Directory, "s2009g", "%02d");
  	GeoidFileNames(15, 15, 0, Directory, "s2009s", "%02d");
  	GeoidFileNames(16, 16, 0, Directory, "s2009p", "%02d");
   	break;  

    case GEOID_USGG2012:
	NumberOfGeoidFiles = 16;

	/*
	| CONUS - Attempt to open the one file model.
	*/
	GeoidFileName(Directory, "s2012u00", FileName);
	if ((FilePointer = fopen(FileName, "rb")))
	{
	    fclose(FilePointer);
	    FilePointer = NULL;
            NumberOfGeoidFiles -= 7;
            strcpy(FileNameArray[Offset++], FileName);
        }
	else
	{
    	    /*
	    | First 8 files are CONUS.
	    */
	    GeoidFileNames(1, 8, Offset, Directory, "s2012u", "");
	    Offset += 8;
        }

        /*
	| Alaska - Attempt to open the one file model.
	*/
	GeoidFileName(Directory, "s2012a00", FileName);
	if ((FilePointer = fopen(FileName, "rb")))
	{
	    fclose(FilePointer);
	    FilePointer = NULL;
	    NumberOfGeoidFiles -= 3;
	    strcpy(FileNameArray[Offset++], FileName);
	}
	else
	{
	    /*
	    | Next 4 files are ALASKA.
	    */
	    GeoidFileNames(1, 4, Offset, Directory, "s2012a", "%02d");
	    Offset += 4;
        }

        /*
	| Next 1 file is HAWAII.
	| Next 1 file is GUAM - NORTHERN MARIANAS.
  	| Next 1 file is SAMOA.
	| Next 1 file is PR/VI.
       	*/
	GeoidFileNames(1, 1, Offset++, Directory, "s2012h00", "");
	GeoidFileNames(1, 1, Offset++, Directory, "s2012g00", "");
	GeoidFileNames(1, 1, Offset++, Directory, "s2012s00", "");
	GeoidFileNames(1, 1, Offset,   Directory, "s2012p00", "");
	break;

    case GEOID_XXUSG:
	NumberOfGeoidFiles = 8;
	GeoidFileNames(1, 8, 0, Directory, "xxusgu", "%02d");
	break;
    }

    /*
    | Open all the files that were found.
    */
    for (n = 0; n < NumberOfGeoidFiles; n++)
    {
	FilePointer = fopen(FileNameArray[n], "rb");
	FilePointerArray[n] = FilePointer;
        if (FilePointer)
	{
	    FilePointer = NULL;
	    FilesOpened++;
	    trace( 3, "GEOID12A: Found and opened %s grid file: %s\n",
		   GeoidName(Geoid), FileNameArray[n] );
 	}
	else
	    trace( 2, "GEOID12A: Unable to open %s grid file: %s\n",
		   GeoidName(Geoid), FileNameArray[n] );
    }

    /*
    | Make sure at least one geoid grid file was found.
    */
    if (!FilesOpened)
    {
	trace(2, "GEOID12A: No geoid grid files found in directory: %s\n", Directory);
	return (FALSE);
    }

    CurrentGeoid = Geoid;
    return (TRUE);
}

/*
| Function: ReadGeoidFileHeaders
| Purpose:  Read headers of all open geoid files.
| Authors:  Daniel A. Cook
|
| Formal Parameters: 
|
|   <none>
|
| Implicit Inputs:
|
|   NumberOfGeoidFiles
|   FilePointerArray
|   FileHeaderArray
|   Geoid grid data files header data
|
| Implicit Outputs:
|
|   FileHeaderArray
|
| Return Value:
|
|   TRUE  = Success
|   FALSE = Fail
|
| Design Issues:
|
|   Null file pointer array entries are left in the file pointer array
|   when the corresponding file fails to be successfully opened. These
|   are skipped over when scanning through the array.
*/
static int ReadGeoidFileHeaders(void)
{
    int n;

    for (n = 0; n < NumberOfGeoidFiles; n++)
    {
	FILE *FilePointer = FilePointerArray[n];

        if (FilePointer)
	{
	    fseek(FilePointer,  0L, SEEK_SET);
            fread((char*) &FileHeaderArray[n].LatMin,   sizeof(double), 1, FilePointer);
	    fread((char*) &FileHeaderArray[n].LonMin,   sizeof(double), 1, FilePointer);
	    fread((char*) &FileHeaderArray[n].LatDelta, sizeof(double), 1, FilePointer);
	    fread((char*) &FileHeaderArray[n].LonDelta, sizeof(double), 1, FilePointer);
	    fread((char*) &FileHeaderArray[n].LatNum,   sizeof(int),    1, FilePointer);
	    fread((char*) &FileHeaderArray[n].LonNum,   sizeof(int),    1, FilePointer);
	    fread((char*) &FileHeaderArray[n].Endian,   sizeof(int),    1, FilePointer);
	 
	    /*
	    | Swap bytes as needed.
	    */
	    if (FileHeaderArray[n].Endian != 1)
	    {
		Swap8(&FileHeaderArray[n].LatMin);
		Swap8(&FileHeaderArray[n].LonMin);
		Swap8(&FileHeaderArray[n].LatDelta);
		Swap8(&FileHeaderArray[n].LonDelta);
		Swap4(&FileHeaderArray[n].LatNum);
		Swap4(&FileHeaderArray[n].LonNum);
		/* Do not swap Endian. */
	    } 
        }
    }

    return (TRUE);
}

/*
| Function: SelectBestGeoidFile
| Purpose:  Select the best grid file to use.
| Authors:  Daniel A. Cook
|
| Formal Parameters: 
|
|   BestGeoidFile = Best grid file to use [Output]
|   Latitude      = Latitude  (degrees)   [Input]
|   Longitude     = Longitude (degrees)   [Input]
|
| Implicit Inputs:
|
|   CurrentGeoid
|   NumberOfGeoidFiles
|   FileNameArray
|   FilePointerArray
|   FileHeaderArray 
|
| Implicit Outputs:
|
|   <none>
|
| Return Value:
|
|   TRUE  = Succes
|   FALSE = Failure
|
| Design Issues:
|
|   A best grid file of "N" means the "Nth" file in the array should be used.
|
|   For the GEOID99 models Alaska and CONUS overlap, and this code forces a
|   "CONUS wins" scenario when interpolating the geoid at points in the overlap
|   region.
*/
static int SelectBestGeoidFile(int *BestGeoidFile, double Latitude, double Longitude)
{
    double LatMin, LatMax, LatDelta, LonMin, LonMax, LonDelta;
    int n, ne, se, we, ee, Rank[64];

    /*
    | This is the Alaska/CONUS overlap exception code.
    | The northwest CONUS grid extends into the Alaska panhandle.
    | The southeast Alaska grid is well north of Washinton state.
    | If in overlap region, then use Alaska data for continuity
    | in the panhandle.
    */
    if ((CurrentGeoid == GEOID_GEOID03)  ||
	(CurrentGeoid == GEOID_GEOID09)  ||
	(CurrentGeoid == GEOID_GEOID99)  ||
	(CurrentGeoid == GEOID_GEOID12A) ||
	(CurrentGeoid == GEOID_USGG2003) ||
	(CurrentGeoid == GEOID_USGG2009) ||
	(CurrentGeoid == GEOID_USGG2012))
    { 
	if ((Latitude  >= 49.0)  && (Latitude  <= 58.0) &&
	    (Longitude <= 234.0) && (Longitude >= 230.0))
	{
	    for (n = 0; n < NumberOfGeoidFiles; n++)
	    {
		if (!strcmp("GEOID09_ak.bin", FileNameArray[n]) ||
		    !strcmp("g2012aa0.bin", FileNameArray[n])   ||
		    !strcmp("s2012a00.bin", FileNameArray[n])   ||
		    !strncmp("a04.bin", &FileNameArray[n][5], 7))
		{
		    *BestGeoidFile = n;
		    return (TRUE);
		}
            }

	    trace( 2, "GEOID12A: In Alaska overlap region. "
		      "Need Southeast Alaska grid file.\n" );

	    return (FALSE);
        }
    }

    /*
    | Scan through all *open* geoid grid files, and *RANK* them
    | The file with the highest RANK decides which grid file we use.
    |
    | Here's how the ranking goes:
    |
    |   0 = Point does not lie in this grid file's area.
    |   1 = Point lies in this file, but at a corner.
    |   2 = Point lies in this file, but at an edge.
    |   3 = Point lies in this file, away from corners/edges.
    | 
    | If a rank 3 or 4 file is found, that file's file number is returned otherwise
    | the file with the highest rank (2 or 1) is returned. We return -1 if all files
    | have rank 0.
    */
    for (n = 0; n < NumberOfGeoidFiles; n++)
    {
	if (FilePointerArray[n])
	{
            Rank[n] = 0;

            LatMin   = FileHeaderArray[n].LatMin;
            LatDelta = FileHeaderArray[n].LatDelta;
            LatMax   = LatMin + LatDelta * (FileHeaderArray[n].LatNum - 1);

            LonMin   = FileHeaderArray[n].LonMin;
            LonDelta = FileHeaderArray[n].LonDelta;
            LonMax   = LonMin + LonDelta * (FileHeaderArray[n].LonNum - 1);

            if ((Latitude  <= LatMax) && (Latitude  >= LatMin) &&
		(Longitude <= LonMax) && (Longitude >= LonMin))
	    {
                /* At this point, we're inside a grid */

                /*
		| 6x6 spline test.
		*/
                if (((Latitude  - LatMin )   > (2.0 * LatDelta)) &&
		    ((LatMax    - Latitude)  > (2.0 * LatDelta)) &&
		    ((Longitude - LonMin )   > (2.0 * LonDelta)) &&
		    ((LonMax    - Longitude) > (2.0 * LonDelta)))
		{
		    *BestGeoidFile = n;
		    return (TRUE);  /* best condition - return right away */
                }

		/*
		| 4x4 spline test.
		*/
		else if (((Latitude  - LatMin)    > LatDelta) &&
			 ((LatMax    - Latitude)  > LatDelta) &&
			 ((Longitude - LonMin)    > LonDelta) &&
			 ((LonMax    - Longitude) > LonDelta))
		{
		    Rank[n] = 3;
                }

		/*
		| Edge and corner test.
		*/
		else
		{
		    /*
		    | Find which of the 9 possible places this point resides:
		    | NW corner, North Edge, NE corner
		    | West Edge, Center    , East edge
		    | SW corner, South Edge, SE corner
 		    */
                    ne = se = we = ee = FALSE;
 
		    /*
		    | Near North edge.
		    */
		    if ((LatMax - Latitude) <= LatDelta)
			ne = TRUE;

		    /*
		    | Near South edge.
		    */
		    if ((Latitude - LatMin) <= LatDelta)
			se = TRUE;
     
		    /*
		    | Near East edge.
		    */
		    if ((LonMax - Longitude) <= LonDelta)
			ee = TRUE;

		    /*
		    | Near West edge.
		    */
		    if ((Longitude - LonMin) <= LonDelta)
			we = TRUE;

                    /*
		    | Set the rank of this file, based on edge-logic.
		    */
		    if (((ne || se) && !(we || ee)) ||
			((we || ee) && !(ne || se)))
		    {
			Rank[n] = 2;
		    }

	 	    if ((ne && we) || (se && we) ||
			(se && ee) || (ne && ee))
		    {
			Rank[n] = 1;
		    }
                }
            }
        }
    }

    /*
    | If we reach this point, all possible files have been searched, 
    | and there's no open file which had a rank of 4. So now, see if
    | we have any rank 3, 2 or 1 files to use.
    */
    for (n = 0; n < NumberOfGeoidFiles; n++)
    {
        if (Rank[n] == 3)
	{
	    *BestGeoidFile = n;
            return (TRUE);
	}
    }

    for (n = 0; n < NumberOfGeoidFiles; n++)
    {
        if (Rank[n] == 2)
	{
	    *BestGeoidFile = n;
            return (TRUE);
	}
    }

    for (n = 0; n < NumberOfGeoidFiles; n++)
    {
        if (Rank[n] == 1)
	{
	    *BestGeoidFile = n;
            return (TRUE);
	}
    }

    /*
    | If we reach here, then no files are acceptable for the given
    | latitude / longitude pair.
    */
    trace( 2, "GEOID12A: no %s grid files with data for (%d, %d) found.\n",
	   GeoidName(CurrentGeoid), Latitude, Longitude );
    return (FALSE);
}

/*
| Function: Spline
| Purpose:  Fast one-dimensional equidistant spline interpolation function
| Authors:  Rene Forsberg
|
| Formal Parameters: 
|
|   X          = Interpolation argument                          [Input]
|   YY         = Data values                                     [Input]
|   WindowSize = Spline Windowsize                               [Input]
|   RR         = Spline moments calculated by InitializeSpline() [Input]
|
| Implicit Inputs:
|
|   <none>
|
| Implicit Outputs:
|
|   <none>
|
| Return Value:
|
|   Spline result.
|
| Design Issues:
|
|   Outside the range, linear extrapolation is used.
|
| References:
|
|   JOSEF STOER: EINFUHRUNG IN DIE NUMERISCHE MATHEMATIK, I
|   SPRINGER 1972, PAGE 81.
*/
static double Spline(double X, double *YY, int WindowSize, double *RR)
{
    int     JJ;
    double  XX, spline;

    if (X < 0)
    {
	/*
	| Lower end extrapolation.
	*/
	spline = YY[0] + (X-2) * (YY[1] - YY[0] - RR[1] / 6.0);
    }
    else if (X >= WindowSize)
    {
	/*
        | Upper end extrapolation.
	*/
	spline = YY[WindowSize-1] + (X-WindowSize-1.) * (YY[WindowSize-1] - YY[WindowSize-2] + RR[WindowSize-2]/6.);
    }
    else
    {
	/*
	| Interpolation.
	*/
	JJ  = (int) X;
        XX = X - JJ;
        spline = YY[JJ-1] + XX * ((YY[JJ]-YY[JJ-1]-RR[JJ-1]/3.-RR[JJ]/6.) 
                          + XX * (RR[JJ-1]/2. 
                          + XX * (RR[JJ]-RR[JJ-1])/6.));
    }

    return (spline);
}

/*
| Function: Spline4
| Purpose:  Spline interpolation using a 4x4 window around the given point
| Authors:  Daniel A. Cook
|
| Formal Parameters: 
|
|   Latitude  = North latitude  of point [Input]
|   Longitude = East  longitude of point [Input]
|   GeoidFile = Grid file number         [Input]
|
| Implicit Inputs:
|
|   FilePointerArray
|   FileHeaderArray 
|   Geoid data file content.
|
| Implicit Outputs:
|
|   <none>
|
| Return Value:
|   
|   Interpolated geoid height.
|
| Design Issues:
|
|   Outside the range, linear extrapolation is used.
|
|   The size of the window can be changed to any even number.
|   Odd numbers are excluded since their use leads to non-symmetry 
|   in the distribution of data arount the interpolation point.
|   Point in question must be inside of the outermost data ring. 
*/
static double Spline4(double Latitude, double Longitude, int GeoidFile)
{
    const int WindowSize = 4; /* size of interpolation window */
    double   LatMin, LonMin, LatDelta, LonDelta;
    double   ColCounter, RowCounter;
    double   AA[4],HC[4], QQ[4], RR[4];
    float    f1,  f2,  f3,  f4, f5,  f6,  f7,  f8;
    float    f9,  f10, f11, f12, f13, f14, f15, f16;
    float    xx, yy;
    long     LonColNum, IColN, IRowN, IRec;
    int      Endian;  
    FILE*    FilePointer;

    /*
    | Define some necessary parameters.
    */
    FilePointer = FilePointerArray[GeoidFile];
    LatMin      = FileHeaderArray[GeoidFile].LatMin;
    LonMin      = FileHeaderArray[GeoidFile].LonMin;
    LatDelta    = FileHeaderArray[GeoidFile].LatDelta;
    LonDelta    = FileHeaderArray[GeoidFile].LonDelta;
    LonColNum   = FileHeaderArray[GeoidFile].LonNum;
    Endian      = FileHeaderArray[GeoidFile].Endian;
 
    /*
    | Find the row / column of the nearest grid node to the latitude / longitude point.
    | This grid node is Southwest from the latitude / longitude point.
    | (Rowcounter, ColCounter) = exact (latitude, longitude) grid coord location (real*8)
    | (Row, Column)            = closest node (to sw) grid coord loc'n (int)
    */
    RowCounter = ((Latitude  - LatMin) / LatDelta);
    ColCounter = ((Longitude - LonMin) / LonDelta);

    /*
    | Find the reference node row and column.
    */
    IRowN = (long) floor(RowCounter);	/* the row just South from Latitude  */
    IColN = (long) floor(ColCounter);	/* the col just West  from Longitude */

    /*
    | Find relative location of the point in the interpolation window.
    */
    yy = (RowCounter - IRowN) + 2.0;	/* 2 := 4x4 spline window */
    xx = (ColCounter - IColN) + 2.0;

    /*
    | At this point, the (irown,icoln) coordinate pair represent 
    | that grid node which is south and west from the lat/lon point, 
    | about which we must get a 4x4 data matrix for spline interpolation.
    |
    | Extract 4x4 array and use it for the spline interpolation:
    |
    |   44L                gets past the header
    |   IRowN * LonColNum  gets to the row
    |   IColN              gets the data from the specific column
    */
    IRec = 44L + (long) (4 * ((IRowN - 1) * LonColNum + (IColN - 1)));
    fseek(FilePointer, IRec, SEEK_SET);
    fread((char*) &f1,  sizeof(float), 1, FilePointer);	/* IColN - 1 */
    fread((char*) &f2,  sizeof(float), 1, FilePointer);	/* IColN     */
    fread((char*) &f3,  sizeof(float), 1, FilePointer);	/* IColN + 1 */
    fread((char*) &f4,  sizeof(float), 1, FilePointer);	/* IColN + 2 */

    IRec = 44L + (long) (4 * (IRowN * LonColNum + (IColN - 1)));
    fseek(FilePointer, IRec, SEEK_SET);
    fread((char*) &f5,  sizeof(float), 1, FilePointer);
    fread((char*) &f6,  sizeof(float), 1, FilePointer);
    fread((char*) &f7,  sizeof(float), 1, FilePointer);
    fread((char*) &f8,  sizeof(float), 1, FilePointer);

    IRec = 44L + (long) (4 * ((IRowN + 1) * LonColNum + (IColN - 1)));
    fseek(FilePointer, IRec, SEEK_SET);
    fread((char*) &f9,  sizeof(float), 1, FilePointer);
    fread((char*) &f10, sizeof(float), 1, FilePointer);
    fread((char*) &f11, sizeof(float), 1, FilePointer);
    fread((char*) &f12, sizeof(float), 1, FilePointer);

    IRec = 44L + (long) (4 * ((IRowN + 2) * LonColNum + (IColN - 1)));
    fseek(FilePointer, IRec, SEEK_SET);
    fread((char*) &f13, sizeof(float), 1, FilePointer);
    fread((char*) &f14, sizeof(float), 1, FilePointer);
    fread((char*) &f15, sizeof(float), 1, FilePointer);
    fread((char*) &f16, sizeof(float), 1, FilePointer);

    /*
    | Swap bytes as needed.
    */
    if (Endian != 1)
    {   
        Swap4(&f1);
        Swap4(&f2);
        Swap4(&f3);
        Swap4(&f4);
        Swap4(&f5);
        Swap4(&f6);
        Swap4(&f7);
        Swap4(&f8);
        Swap4(&f9);
        Swap4(&f10);
        Swap4(&f11);
        Swap4(&f12);
        Swap4(&f13);
        Swap4(&f14);
        Swap4(&f15);
        Swap4(&f16);
    }

    AA[0] = f1;
    AA[1] = f2;
    AA[2] = f3;
    AA[3] = f4;
    InitializeSpline(AA, WindowSize, RR, QQ);
    HC[0] = Spline(xx, AA, WindowSize, RR);

    AA[0] = f5;
    AA[1] = f6;
    AA[2] = f7;
    AA[3] = f8;
    InitializeSpline(AA, WindowSize, RR, QQ);
    HC[1] = Spline(xx, AA, WindowSize, RR);

    AA[0] = f9;
    AA[1] = f10;
    AA[2] = f11;
    AA[3] = f12;
    InitializeSpline(AA, WindowSize, RR, QQ);
    HC[2] = Spline(xx, AA, WindowSize, RR);

    AA[0] = f13;
    AA[1] = f14;
    AA[2] = f15;
    AA[3] = f16;
    InitializeSpline(AA, WindowSize, RR, QQ);
    HC[3] = Spline(xx, AA, WindowSize, RR);
  
    InitializeSpline(HC, WindowSize, RR, QQ);
    return (Spline(yy, HC, WindowSize, RR));
}

/*
| Function: Spline6
| Purpose:  Spline interpolation using a 6x6 window around the given point
| Authors:  Daniel A. Cook
|
| Formal Parameters: 
|
|   Latitude         = North latitude  of point [Input]
|   Longitude        = East  longitude of point [Input]
|   GeoidFile        = Grid file number         [Input]
|
| Implicit Inputs:
|
|   FilePointerArray
|   FileHeaderArray 
|   Geoid data file content.
|
| Implicit Outputs:
|
|   <none>
|
| Return Value:
|   
|   Interpolated geoid height.
|
| Design Issues:
|
|   Outside the range, linear extrapolation is used.
|
|   The size of the window can be changed to any even number.
|   Odd numbers are excluded since their use leads to non-symmetry 
|   in the distribution of data arount the interpolation point.
|   Point in question must be inside of the outermost data ring. 
*/
static double Spline6(double Latitude, double Longitude, int GeoidFile)
{
    const int WindowSize = 6; /* size of interpolation window */
    double   LatMin, LatDelta, LonMin, LonDelta, ColCounter, RowCounter;
    double   AA[6],HC[6], QQ[6], RR[6];
    float    f1,  f2,  f3,  f4, f5,  f6,  f7,  f8, f9,  f10, f11, f12;
    float    f13, f14, f15, f16, f17, f18, f19, f20, f21, f22, f23, f24;
    float    f25, f26, f27, f28, f29, f30, f31, f32, f33, f34, f35, f36;
    float    xx, yy;
    long     LonColNum, IColN, IRowN, IRec;
    int      Endian;  
    FILE*    FilePointer;

    /*
    | Define some necessary parameters.
    */
    FilePointer = FilePointerArray[GeoidFile];
    LatMin      = FileHeaderArray[GeoidFile].LatMin;
    LonMin      = FileHeaderArray[GeoidFile].LonMin;
    LatDelta    = FileHeaderArray[GeoidFile].LatDelta;
    LonDelta    = FileHeaderArray[GeoidFile].LonDelta;
    LonColNum   = FileHeaderArray[GeoidFile].LonNum;
    Endian      = FileHeaderArray[GeoidFile].Endian;
 
    /*
    | Find the row / column of the nearest grid node to the latitude / longitude point.
    | This grid node is Southwest from the latitude / longitude point.
    | (Rowcounter, ColCounter) = exact (latitude, longitude) grid coord location (real*8)
    | (Row, Column)            = closest node (to sw) grid coord loc'n (int)
    */
    RowCounter = ((Latitude  - LatMin) / LatDelta);
    ColCounter = ((Longitude - LonMin) / LonDelta);

    /*
    | Find the reference node row and column.
    */
    IRowN = (long) floor(RowCounter);	/* the row just South from Latitude  */
    IColN = (long) floor(ColCounter);	/* the col just West  from Longitude */

    /*
    | Find relative location of the point in the interpolation window.
    */
    yy = (RowCounter - IRowN) + 3.0;	/* 3 := 6x6 spline window */
    xx = (ColCounter - IColN) + 3.0;

    /*
    | At this point, the (irown,icoln) coordinate pair represent 
    | that grid node which is south and west from the lat/lon point, 
    | about which we must get a 6x6 data matrix for spline interpolation.
    |
    | Extract 6x6 array and use it for the spline interpolation:
    |
    |   44L                gets past the header
    |   IRowN * LonColNum  gets to the row
    |   IColN              gets the data from the specific column
    */
    IRec = 44L + (long) (4 * ((IRowN-2) * LonColNum + (IColN - 2)));
    fseek(FilePointer, IRec, SEEK_SET);
    fread((char*) &f1,  sizeof(float), 1, FilePointer);    /* IColN - 2 */
    fread((char*) &f2,  sizeof(float), 1, FilePointer);    /* IColN - 1 */
    fread((char*) &f3,  sizeof(float), 1, FilePointer);    /* IColN     */
    fread((char*) &f4,  sizeof(float), 1, FilePointer);    /* IColN + 1 */
    fread((char*) &f5,  sizeof(float), 1, FilePointer);    /* IColN + 2 */
    fread((char*) &f6,  sizeof(float), 1, FilePointer);    /* IColN + 3 */

    IRec = 44L + (long) (4* ((IRowN-1) * LonColNum + (IColN - 2)));
    fseek(FilePointer, IRec, SEEK_SET);
    fread((char*) &f7,  sizeof(float), 1, FilePointer);
    fread((char*) &f8,  sizeof(float), 1, FilePointer);
    fread((char*) &f9,  sizeof(float), 1, FilePointer);
    fread((char*) &f10, sizeof(float), 1, FilePointer);
    fread((char*) &f11, sizeof(float), 1, FilePointer);
    fread((char*) &f12, sizeof(float), 1, FilePointer);

    IRec = 44L + (long) (4 * (IRowN * LonColNum + (IColN - 2)));
    fseek(FilePointer, IRec, SEEK_SET);
    fread((char*) &f13, sizeof(float), 1, FilePointer);
    fread((char*) &f14, sizeof(float), 1, FilePointer);
    fread((char*) &f15, sizeof(float), 1, FilePointer);
    fread((char*) &f16, sizeof(float), 1, FilePointer);
    fread((char*) &f17, sizeof(float), 1, FilePointer);
    fread((char*) &f18, sizeof(float), 1, FilePointer);

    IRec = 44L + (long) (4 * ((IRowN + 1) * LonColNum + (IColN - 2)));
    fseek(FilePointer, IRec, SEEK_SET);
    fread((char*) &f19, sizeof(float), 1, FilePointer);
    fread((char*) &f20, sizeof(float), 1, FilePointer);
    fread((char*) &f21, sizeof(float), 1, FilePointer);
    fread((char*) &f22, sizeof(float), 1, FilePointer);
    fread((char*) &f23, sizeof(float), 1, FilePointer);
    fread((char*) &f24, sizeof(float), 1, FilePointer);

    IRec = 44L + (long) (4 * ((IRowN + 2) * LonColNum + (IColN - 2)));
    fseek(FilePointer, IRec, SEEK_SET);
    fread((char*) &f25, sizeof(float), 1, FilePointer);
    fread((char*) &f26, sizeof(float), 1, FilePointer);
    fread((char*) &f27, sizeof(float), 1, FilePointer);
    fread((char*) &f28, sizeof(float), 1, FilePointer);
    fread((char*) &f29, sizeof(float), 1, FilePointer);
    fread((char*) &f30, sizeof(float), 1, FilePointer);

    IRec = 44L + (long) (4 * ((IRowN + 3) * LonColNum + (IColN - 2)));
    fseek(FilePointer, IRec, SEEK_SET);
    fread((char*) &f31, sizeof(float), 1, FilePointer);
    fread((char*) &f32, sizeof(float), 1, FilePointer);
    fread((char*) &f33, sizeof(float), 1, FilePointer);
    fread((char*) &f34, sizeof(float), 1, FilePointer);
    fread((char*) &f35, sizeof(float), 1, FilePointer);
    fread((char*) &f36, sizeof(float), 1, FilePointer);

    /*
    | Swap bytes as needed.
    */
    if (Endian != 1)
    {
        Swap4(&f1);
        Swap4(&f2);
        Swap4(&f3);
        Swap4(&f4);
        Swap4(&f5);
        Swap4(&f6);
        Swap4(&f7);
        Swap4(&f8);
        Swap4(&f9);
        Swap4(&f10);
        Swap4(&f11);
        Swap4(&f12);
        Swap4(&f13);
        Swap4(&f14);
        Swap4(&f15);
        Swap4(&f16);
        Swap4(&f17);
        Swap4(&f18);
        Swap4(&f19);
        Swap4(&f20);
        Swap4(&f21);
        Swap4(&f22);
        Swap4(&f23);
        Swap4(&f24);
        Swap4(&f25);
        Swap4(&f26);
        Swap4(&f27);
        Swap4(&f28);
        Swap4(&f29);
        Swap4(&f30);
        Swap4(&f31);
        Swap4(&f32);
        Swap4(&f33);
        Swap4(&f34);
        Swap4(&f35);
        Swap4(&f36);
    }

    AA[0] = f1;
    AA[1] = f2;
    AA[2] = f3;
    AA[3] = f4;
    AA[4] = f5;
    AA[5] = f6;
    InitializeSpline(AA, WindowSize, RR, QQ);
    HC[0] = Spline(xx, AA, WindowSize, RR);

    AA[0] = f7;
    AA[1] = f8;
    AA[2] = f9;
    AA[3] = f10;
    AA[4] = f11;
    AA[5] = f12;
    InitializeSpline(AA, WindowSize, RR, QQ);
    HC[1] = Spline(xx, AA, WindowSize, RR);
 
    AA[0] = f13;
    AA[1] = f14;
    AA[2] = f15;
    AA[3] = f16;
    AA[4] = f17;
    AA[5] = f18;
    InitializeSpline(AA, WindowSize, RR, QQ);
    HC[2] = Spline(xx, AA, WindowSize, RR);
 
    AA[0] = f19;
    AA[1] = f20;
    AA[2] = f21;
    AA[3] = f22;
    AA[4] = f23;
    AA[5] = f24;
    InitializeSpline(AA, WindowSize, RR, QQ);
    HC[3] = Spline(xx, AA, WindowSize, RR);

    AA[0] = f25;
    AA[1] = f26;
    AA[2] = f27;
    AA[3] = f28;
    AA[4] = f29;
    AA[5] = f30;
    InitializeSpline(AA, WindowSize, RR, QQ);
    HC[4] = Spline(xx, AA, WindowSize, RR);

    AA[0] = f31;
    AA[1] = f32;
    AA[2] = f33;
    AA[3] = f34;
    AA[4] = f35;
    AA[5] = f36;
    InitializeSpline(AA, WindowSize, RR, QQ);
    HC[5] = Spline (xx, AA, WindowSize, RR);

    InitializeSpline(HC, WindowSize, RR, QQ);
    return (Spline(yy, HC, WindowSize, RR));
}

/*
| Function: Swap4
| Purpose:  Endian swap four bytes
| Authors:  Daniel A. Cook
|
| Formal Parameters: 
|
|   P = Input pointer [Input]
|
| Implicit Inputs:
|
|   <none>
|
| Implicit Outputs:
|
|   <none>
|
| Return Value:
|
|   <none>
|
| Design issues:
|
*/
static void Swap4(void *p)
{
    unsigned char t;
    unsigned char *uc = (unsigned char*) p;

    t = uc[0]; uc[0] = uc[3]; uc[3] = t;
    t = uc[1]; uc[1] = uc[2]; uc[2] = t;	
}

/*
| Function: Swap8
| Purpose:  Endian swap eight bytes
| Authors:  Daniel A. Cook
|
| Formal Parameters: 
|
|   P = Input pointer [Input]
|
| Implicit Inputs:
|
|   <none>
|
| Implicit Outputs:
|
|   <none>
|
| Return Value:
|
|   <none>
|
| Design issues:
|
*/
static void Swap8(void *p)
{
    unsigned char t;
    unsigned char *uc = (unsigned char*) p;

    t = uc[0]; uc[0] = uc[7]; uc[7] = t;
    t = uc[1]; uc[1] = uc[6]; uc[6] = t;
    t = uc[2]; uc[2] = uc[5]; uc[5] = t;
    t = uc[3]; uc[3] = uc[4]; uc[4] = t;
}