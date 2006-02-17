/*
 * Author: Torres C. (Univ of TX at Brownsville)
 */
/*
 * This code is only for TESTING 
 * creates simulation frames
 */
#include "TSDatagen.h"
#include "tracksearch.h"
#include <config.h>
#include <lal/FrameStream.h>
#include <lal/FrequencySeries.h>
#include <lal/LALConstants.h>
#include <lal/LALNoiseModels.h>
#include <stdio.h>
#include <string.h>

#define PROGRAM_NAME "TSDatagen"

typedef struct
{
  INT4    argc;
  CHAR**  argv;
}LALInitSearchParams;

/* Declare Local Subroutines Here */
int      intializeArgs(int argc, char* argv[],TSDataGenParams *params);
void        createdata(LALStatus*,REAL4Vector*,REAL4Vector*,TSDataGenParams);
void    generateoutput(LALStatus*,REAL4Vector*,REAL4Vector*,TSDataGenParams);
void          writePSD(LALStatus*,TSDataGenParams);
void Get_External_Data(LALStatus*,REAL4Vector*,REAL4Vector*,TSDataGenParams);
void multipleInjection(LALStatus*,REAL4Vector*,INT4,INT4);

/* Code identification Text */
NRCSID( TRACKSEARCHC, "TSDatagen $Id$");
RCSID( "datagen $Id$");
#define CVS_REVISION "$Revision$"
#define CVS_SOURCE "$Source$"
#define CVS_DATE "$Date$"

#define TRUE     1
#define FALSE    0

#ifndef HAVE_LIBLALFRAME
int main( void )
{
  fputs( "Disabled: LALApps compiled with non-frame-enabled LAL\n", stderr );
  return 77;
}
#
#else
#

/* Usage format string */
#define USAGE "Unknown"

/* Code ideas */
/* Take in Start Amplitude
   Finishd Amplitude
   Start Frequency
   Finish Frequency
   Sampling Frequency
   Noise Amplitude
*/

/* Code Body */
int main (int argc, char *argv[])
{
  TSDataGenParams                params;
  static LALStatus               status;
  REAL4Vector                   *dataX=NULL;
  REAL4Vector                   *dataY=NULL;
  /*
   *Sleep for Attaching DDD 
   */
  unsigned int doze = 0;
  pid_t myPID;
  myPID = getpid( );
  fprintf( stdout, "pid %d sleeping for %d seconds\n", myPID, doze );
  fflush( stdout );
  sleep( doze );
  fprintf( stdout, "pid %d awake\n", myPID );
  fflush( stdout );
  /* End Global Variable Declarations */
   
  /* Debug Settings */
  /* SET LAL DEBUG STUFF */
  set_debug_level("MEMDBG");

  intializeArgs(argc,argv,&params);
  /* Prepare to make frame time series */
  /* If default params left untouched we skip this */
  if ((params.quiet != params.noiseOnly) || (params.externalSignal))
    {
      LALCreateVector(&status,&dataX,params.numSamplePoints);
      LALCreateVector(&status,&dataY,params.numSamplePoints);
      createdata(&status,dataX,dataY,params);
      generateoutput(&status,dataX,dataY,params);
      LALDestroyVector(&status,&dataY);
      LALDestroyVector(&status,&dataX);
      if (params.signalFileName)
	LALFree(params.signalFileName);
      if (params.name)
	LALFree(params.name);
    }
  /*Make and write out the PSD */
  /*If default arguments changed by user */
  if ((params.noisePSDMaxF != 0) && (params.noisePSDDeltaF != 0))
    writePSD(&status,params);
  /* Free all memory allocated inside params(TSDataGenParams) struct*/
  /* during paramter initialization etc */
  /*Check for memory leaks*/
  LALCheckMemoryLeaks();
  return 0;
}/*End Routine*/


int intializeArgs(
		  int         argc,
		  char       *argv[],
		  TSDataGenParams *params
		  )
{
  /* getop arguments */
  struct option long_options[] = 
    {
      {"Amplitude_Initial",      required_argument,      0,       'a'},
      {"Amplitude_Final",        required_argument,      0,       'b'},
      {"Start_Frequency",        required_argument,      0,       'c'},
      {"Finish_Frequency",       required_argument,      0,       'd'},
      {"Sampling_Frequency",     required_argument,      0,       'e'},
      {"Sample_Points",          required_argument,      0,       'g'},
      {"Noise_Amplitude",        required_argument,      0,       'f'},
      {"File_Name",              required_argument,      0,       'h'},
      {"Quiet",                  no_argument,            0,       'i'},
      {"SNR",                    required_argument,      0,       'j'},
      {"Noise_Only",             no_argument,            0,       'k'},
      {"External_Signal",        required_argument,      0,       'l'},
      {"Seed",                   required_argument,      0,       'm'},
      {"Multiple_Injections",    required_argument,      0,       'n'},
      {"Multiple_Inject_Spacing",required_argument,      0,       'o'},
      {"LIGOIPSD_MaxF"          ,required_argument,      0,       'p'},
      {"LIGOIPSD_DeltaF"        ,required_argument,      0,       'q'},
      {0,                                        0,      0,         0}
    };

  int                     C;


  /* Set arguements defaut parameter */
  params->ampInitial = 1.0;
  params->ampFinal = 1.0;
  params->freqInitial = 0.0;
  params->freqFinal = 0.0;
  params->sampleFreq = 1.0;
  params->numSamplePoints = 0;
  params->noiseAmp = 1.0;
  params->seed = 0.0;
  params->SNR = 1.0;
  params->quiet = 0;
  params->noiseOnly = 0;
  params->externalSignal = 0;
  params->multipleInjects=0;
  params->multipleInjectSpacing=0;
  params->signalFileName=NULL;
  params->name=NULL;
  params->noisePSDMaxF=0;
  params->noisePSDDeltaF=0;

  if (argc < 1) /* Not enough arguments */
    {
      fprintf(stderr,TSDATAGENC_MSGEARGS);
      fprintf(stderr,"\n");
      exit(TSDATAGENC_EARGS);
    }

  /* Loop through parsing arguments */
  while (TRUE)
    {
      int option_index=0;
      C = getopt_long_only(argc,argv,"a:b:c:d:e:f:g:h:i:j:k:l:m",long_options,&option_index);
      if (C == -1) /* No more arguements to parse */
	{
	  break;
	}
      switch(C)
	{
	case 'a':
	  { /* Setting Amplitude Initial */
	    params->ampInitial = atof(optarg);
	    if ( params->ampInitial < 0) 
	      {
		fprintf(stderr,TSDATAGENC_MSGEVAL);
		exit(TSDATAGENC_EARGS);
	      };
	  }
	  break;
	  
	case 'b':
	  { /* Set finish amplitude of signal */
	    params->ampFinal = atof(optarg);
	    if (params->ampFinal < 0) 
	      {
		fprintf(stderr,TSDATAGENC_MSGEVAL);
		exit(TSDATAGENC_EARGS);
	      };
	  }
	  break;
	  
	case 'c':
	  { /* Settig up sin initial frequency */
	    params->freqInitial = atof(optarg);
	    if (params->freqInitial < 0) 
	      {
		fprintf(stderr,TSDATAGENC_MSGEVAL);
		exit(TSDATAGENC_EARGS);
	      };
	  }
	  break;
	  
	case 'd':
	  { /* Setting up final frequency part */
	    params->freqFinal = atof(optarg);
	    if (params->freqFinal < 0) 
	      {
		fprintf(stderr,TSDATAGENC_MSGEVAL);
		exit(TSDATAGENC_EARGS);
	      };
	  }
	  break;
	  
	case 'e':
	  { /* Getting the sampling frequency */
	    params->sampleFreq = atof(optarg);
	    if (params->sampleFreq < 0)
	      {
		fprintf(stderr,TSDATAGENC_MSGEVAL);
		exit(TSDATAGENC_EARGS);
	      };
	  }
	  break;
	  
	case 'f':
	  { /* Setting relative noise amp number */
	    params->noiseAmp = atoi(optarg);
	    if (params->noiseAmp < 0)
	      {
		fprintf(stderr,TSDATAGENC_MSGEVAL);
		exit(TSDATAGENC_EARGS);
	      };
	  }
	  break;

	case 'g':
	  { /* Read in SamplePoint Count */
	    params->numSamplePoints = atoi(optarg);
	    if (params->numSamplePoints < 0)
	      {
		fprintf(stderr,TSDATAGENC_MSGEVAL);
		exit(TSDATAGENC_EARGS);
	      };
	  }
	  break;

	case 'h':
	  { /* Read in data file name to create */
	    params->name = (CHAR*) LALMalloc(strlen(optarg)+1);
	    strcpy(params->name,optarg);
	  }
	  break;
	  
	case 'i':
	  { /*Checking for request for quiet data*/
	    params->quiet = 1;
	  }
	  break;

	case 'j':
	  { /* Setting user requested SNR for data */
	    params->SNR = atof(optarg);
	  }
	  break;

	case 'k':
	  { /* Setup to output a pure noise data file */
	    params->noiseOnly = 1;
	  }
	  break;

	case 'l':
	  { /* Set flag for external Signal and path/filename */
	    params->signalFileName = (CHAR*) LALMalloc(strlen(optarg)+1);
	    strcpy(params->signalFileName,optarg);
	    params->externalSignal=1;
	  }
	  break;

	case 'm':
	  { /* Set a search seed for testing */
	    params->seed = atoi(optarg);
	  }
	  break;

	case 'n':
	  { /* grab number of copies to make */
	    params->multipleInjects=atoi(optarg);
	  }
	  break;

	case 'o':
	  { /* grab silent points to insert with copies */
	    params->multipleInjectSpacing=atoi(optarg);
	  }
	  break;

	case 'p':
	  { /* Setup Noise PSD Max F for writing to frame */
	    params->noisePSDMaxF=atof(optarg);
	  }
	  break;

	case 'q':
	  { /* Setup Noise PSD freq resolution */
	    params->noisePSDDeltaF=atof(optarg);
	  }
	  break;

	default:
	  {
	    fprintf(stderr,TSDATAGENC_MSGEMISC);
	    exit(TSDATAGENC_EMISC);
	  }
	};
    }; /* End while */

  return 0;  
} /* End Subroutine */


void createdata(
		LALStatus          *status,
		REAL4Vector        *dataX,
		REAL4Vector        *dataY,
		TSDataGenParams     params
		)
{
  REAL4             deltaA;
  REAL8             deltaF;
  REAL8             currentF;
  INT4              i;
  UINT4             j;
  REAL4Vector      *nd = NULL;
  RandomParams     *RP = NULL;
  INITSTATUS (status, "makefakenoise", TRACKSEARCHC);
  ATTATCHSTATUSPTR (status);
  
  /* Define DeltaA for data set */
  deltaA = (params.ampFinal-params.ampInitial)/params.numSamplePoints;
  /* Define DeltaF for data set */
  deltaF = (params.freqFinal-params.freqInitial)/params.numSamplePoints;
  /* The slope deltaF needs to divided by two to abvoid going to */
  /* zero f before reaching end of data vector */
  deltaF = deltaF/2.0; 
  /* Preinitialize data outgoing vectors */
  for (i=0;i<params.numSamplePoints;i++)
    {
      dataY->data[i]=0.0;
      dataX->data[i]=0.0;
    };
  /* Checking for Noise only request */
  if (!params.noiseOnly)
    {
      /* Insert IF Statment here to accept outside signals */
      /* We can assume that they are sampled at unit time interval */
      /* The real information should be sampled at least 2Nyquist */
      if (!params.externalSignal)
	{
	  /*Loop for setting signal into data structure */
	  for ( i = 0; i < params.numSamplePoints; i++)
	    {
	      currentF = params.freqInitial+(i*deltaF);
	      dataY->data[i] = (params.ampInitial+(i*deltaA))
		*(sin(2*LAL_PI*(currentF)*i));
	      dataX->data[i] = (i* (1/params.sampleFreq));
	    };
	}
      else
	{ /* Read in external Signal */
	  Get_External_Data(status->statusPtr,dataX,dataY,params);
	}; /* Done Reading Or Creating UnNormalized Signal */
      /* Check if user wants multiple injects in gwf file */
      /* Copy data n times for multiple injections */
      /* Place a N points space between each injections requested */
      /* */
      /* Set SNR of proposed signal here */
      for ( i = 0; i < params.numSamplePoints; i++)
	{
	  dataY->data[i] = dataY->data[i]*params.SNR;
	};
    }
  else
    {
      /*Use zero pad array to avoid copying noise multiple times */
      for ( i = 0; i < params.numSamplePoints; i++)
	{
	  dataY->data[i]=0;
	};
    };
  if (( params.multipleInjects > 0) &&
      (params.multipleInjectSpacing > 0))
    {
      /* Setup signal structure with multiple injects*/
      /* This routine reallocates ram for multiple insertions */
      multipleInjection(status->statusPtr,dataY,params.multipleInjects,params.multipleInjectSpacing);
      /* Please ignore dataX this is a useless variable*/
    }
    
  /* Add unit noise to signal to achieve desired SNR (white)*/
  if (!params.quiet) 
    {
      LALCreateVector(status->statusPtr,&nd,dataY->length);
      printf("My seed is %i\n",params.seed);
      LALCreateRandomParams(status->statusPtr,&RP,params.seed);
      LALNormalDeviates(status->statusPtr,nd,RP);
      LALDestroyRandomParams(status->statusPtr,&RP);
      /* Setting Perm Noise Amp of 1 */
      params.noiseAmp = 1.0;
      for ( j = 0; j < dataY->length; j++)
	{
	  dataY->data[j] = dataY->data[j] + (params.noiseAmp * nd->data[j]);
	};
      LALDestroyVector(status->statusPtr,&nd);
    };

  DETATCHSTATUSPTR (status);
  return;
}/* End of data setup subroutine */

void generateoutput(
		    LALStatus         *status,
		    REAL4Vector       *dataX,
		    REAL4Vector       *dataY,
		    TSDataGenParams    params
		    )
{

  REAL4TimeSeries       dataTS;
  FrOutPar              frameheader;
  INT4                   j;
  CHAR                   detector[16]="Simulate";
  FILE                  *fp;
  CHAR                  *filetxtname;

  INITSTATUS (status, "makefakedata", TRACKSEARCHC);
  ATTATCHSTATUSPTR (status);

  /* Fill time series struct with pseudo-data */

  /*  dataTS->name = detector; */
  strcpy(dataTS.name,"Simulate");
  dataTS.epoch.gpsSeconds = 0;
  dataTS.epoch.gpsNanoSeconds = 0;
  dataTS.deltaT = 1/params.sampleFreq;
  dataTS.f0 = 0;
  dataTS.sampleUnits = lalADCCountUnit;
  dataTS.data = dataY;

  /* Create frame header */
  /*The output frame file name will be 
    source-description-GPS start time-duration.gwf.*/

  frameheader.source = detector;
  frameheader.description = "FakeTSS";
  frameheader.type = LAL_ADC_CHAN; /*Used so that frame stream can be read */
  frameheader.nframes = 1;
  frameheader.frame = 1;
  frameheader.run = 1;

  /* Call frame writing subroutine */

  LALFrWriteREAL4TimeSeries(status->statusPtr,&dataTS,&frameheader);

  /* Write plaintext equivalent file 2C*/
  filetxtname = (CHAR *) LALCalloc(32,sizeof(CHAR));
  sprintf(filetxtname,"Ascii--%s-%i-%i.txt",
	  frameheader.description,
	  dataTS.epoch.gpsSeconds,
	  dataY->length);
  fp = fopen(filetxtname,"w");
  for (j=0; j < (INT4) dataY->length;j++)
    {
      fprintf(fp,"%f %f\n",j,dataY->data[j]);
    }
  fclose(fp);
  LALFree(filetxtname);
  DETATCHSTATUSPTR (status);
  return;
} /* End Subroutine */

  /* Subroutine to read in data from external Signal's ASCII file */
void Get_External_Data(LALStatus*          status,
		       REAL4Vector        *dataX,
		       REAL4Vector        *dataY,
		       TSDataGenParams     params
		       )
{
  FILE                 *fp=NULL;
  INT4                  i;
  INITSTATUS (status, "readascii", TRACKSEARCHC);
  ATTATCHSTATUSPTR (status);
  /* File opening via an absolute path */
  fp = fopen(params.signalFileName,"r");
  if (!fp)
    {
      fprintf(stderr,TSDATAGENC_MSGEREAD);
      exit(TSDATAGENC_EREAD);
    };
  for(i=0;i<params.numSamplePoints;i++)
    {
      fscanf(fp,"%f\n",&(dataY->data[i]));
      dataX->data[i]=i;
    }
  fclose(fp);
  if (((INT4) dataY->length) != params.numSamplePoints)
    {
      fprintf(stderr,TSDATAGENC_MSGEREAD);
      exit(TSDATAGENC_EREAD);
    }
  DETATCHSTATUSPTR(status);
  return;
}

void multipleInjection(LALStatus*               status,
		       REAL4Vector*            p_dataY,
		       INT4                     nInjects,
		       INT4                     nPoints)
{    
  INT4         i;
  INT4         j;
  INT4         k;
  INT4         l;
  INT4         origLength;
  INT4         totalPoints;
  REAL4Vector* tempVec=NULL;
  INITSTATUS (status, "readascii", TRACKSEARCHC);
  ATTATCHSTATUSPTR (status);
  LALCreateVector(status->statusPtr,&tempVec,p_dataY->length);
  origLength=p_dataY->length;
  /*Copy over data temporary*/
  for (i=0;i<origLength;i++)
    {
      tempVec->data[i]=p_dataY->data[i];
    }
  totalPoints=nInjects*(p_dataY->length+2*nPoints);
  /* ReAllocate new larger vector */
  LALResizeVector(status->statusPtr,&p_dataY,totalPoints);
  /* Copy nInject copies into vector */
  l=0;
  for (i=1;i<=nInjects;i++)
    {
      /* Insert silent points Ahead of waveform*/
      for (k=0;k<nPoints;k++)
	{
	  p_dataY->data[l]=0;
	  l++;
	};
      /*Copy Signal*/
      for (j=0;j<origLength;j++)
	{
	  p_dataY->data[l]=tempVec->data[j];
	  l++;
	};
      /* Insert silent points after waveform*/
      for (k=0;k<nPoints;k++)
	{
	  p_dataY->data[l]=0;
	  l++;
	};
    };
  LALDestroyVector(status->statusPtr,&tempVec);
  DETATCHSTATUSPTR(status);
  return;
}

void writePSD(
	      LALStatus         *status,
	      TSDataGenParams    params
	      )

{
  INT4   j;
  REAL8FrequencySeries      *PSD=NULL;
  LIGOTimeGPS                PSDstamp;
  FrOutPar                   frameheader;
  CHAR                       noisePSDName[16]="LIGOI_PSD";
  FILE                      *fp;
  CHAR                      *filetxtname;

 INITSTATUS (status, "makeLIGO_PSD", TRACKSEARCHC);
 ATTATCHSTATUSPTR (status);
 /* Simple checks */
 ASSERT(params.noisePSDDeltaF > 0,status,TSDATAGENC_EARGS,TSDATAGENC_MSGEVAL);
 ASSERT(params.noisePSDMaxF > 0,status,TSDATAGENC_EARGS,TSDATAGENC_MSGEVAL);
 ASSERT(params.noisePSDMaxF/params.noisePSDDeltaF > 1,status,TSDATAGENC_EARGS,TSDATAGENC_MSGEVAL);
 /* Allocate RAM for structs */
 PSDstamp.gpsSeconds=0;
 PSDstamp.gpsNanoSeconds=0;
 /* Wrong units not important though */
 /* Try using the XLAL code version */
 PSD=XLALCreateREAL8FrequencySeries(noisePSDName,
				    &PSDstamp,
				    0,
				    params.noisePSDDeltaF,
				    &lalHertzUnit,
				    (params.noisePSDMaxF/params.noisePSDDeltaF));
 /*Fill in the series with the appropriate PSD*/
 LALNoiseSpectralDensity(status->statusPtr,
			 PSD->data,
			 LALLIGOIPsd,
			 PSD->deltaF);
 CHECKSTATUSPTR(status);
 /* Create my frame header */
 frameheader.source = noisePSDName;
 frameheader.description = "LIGOI_PSD";
 frameheader.type = LAL_ADC_CHAN; /*Used so that frame stream can be read */
 frameheader.nframes = 1;
 frameheader.frame = 1;
 frameheader.run = 1;
 /* Frame writing subroutine */
 LALFrWriteREAL8FrequencySeries(status->statusPtr,PSD,&frameheader,0);
 /*Write 2C text file for data evaluation */
 filetxtname = (CHAR *) LALCalloc(32,sizeof(CHAR));
  sprintf(filetxtname,"Ascii--%s-%i-%i.txt",
	  frameheader.description,
	  PSD->f0,
	  ((INT4) (PSD->deltaF*PSD->data->length)));
  fp = fopen(filetxtname,"w");
  for (j=0; j < (INT4) PSD->data->length;j++)
    {
      /*  fprintf(fp,"%e %e\n",dataX->data[j],dataY->data[j]); */
      fprintf(fp,"%f %e\n",j*PSD->deltaF,PSD->data->data[j]);
    }
  fclose(fp);
  /* Deallocate Ram */
  LALFree(filetxtname);
  LALDestroyREAL8FrequencySeries(status->statusPtr,PSD);
  DETATCHSTATUSPTR (status);
  return;
}

#endif
