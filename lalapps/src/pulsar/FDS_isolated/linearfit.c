
/* B. Krishnan, July 2005 */
/* small modification by M.A. Papa */
/* $Id$*/
#include <stdio.h>
#include <math.h>
#include <gsl/gsl_fit.h>



int main( int argc, char *argv[]){

  double *x=NULL; /* x values -- here h_0 values */
  double *y=NULL;  /* y values -- here Conf levels */
  double *w=NULL;  /* weights */
  FILE *fp=NULL;
  FILE *fpout=NULL;
  int n=0, r, i; 
  double temp1, temp2, temp3, temp4, temp5;
  double c0, c1, cov00, cov01, cov11, chisq;
  double h0, y0, y0_err, dh0, h0_fit;

  /* open file for reading */
  fp = fopen( argv[1], "r");

  /* read the file to count lines */  
  do{
    r=fscanf(fp,"%lf%lf%lf%lf%lf\n", &temp1, &temp2, &temp3, &temp4, &temp5); 
    if (r==5) {
      n++;
    }
  } while (r != EOF);

  /* go back to start of file */
  rewind(fp);

  /* allocate sufficient memory for x, y and z */
  x = (double *)malloc(sizeof(double)*n);
  if (x==NULL) 
    {
      fprintf(stderr, "unable to allocate memory\n");
      return 1;
    }
  y = (double *)malloc(sizeof(double)*n);
    if (y==NULL) 
    {
      fprintf(stderr, "unable to allocate memory\n");
      return 1;
    }
  w = (double *)malloc(sizeof(double)* n);
  if (w==NULL) 
    {
      fprintf(stderr, "unable to allocate memory\n");
      return 1;
    }

  /* read file again and fill in values of vectors */  
  n = 0;
  do{
     r=fscanf(fp,"%lf%lf%lf%lf%lf\n", &temp1, &temp2, x+n, &temp4, y+n); 
    if (r==5) {
      w[n] = 1.0/(temp2 * temp2); /* weight = 1/variance*/
      n++;
    }
  } while (r != EOF); 

  /* now we are done with the input file */  
  fclose(fp);
  
  /* call gsl fitting function */
  gsl_fit_wlinear(x, 1, w, 1, y, 1, n, &c0, &c1, &cov00, &cov01, &cov11, &chisq); 
  
  h0 = (0.95 - c0)/c1;
  gsl_fit_linear_est(h0, c0, c1, cov00, cov01, cov11, &y0, &y0_err); 

  fprintf(stdout, "h095:  %11.7E\n slope:   %11.7E\n y0_err:   %11.7E\n  h0_err: %11.7E\n", h0* 1.0e-19, c1 * 1.0e+19, y0_err, y0_err/(c1*1.0e+19) );

  fprintf(stdout, "best fit : Y = %g + %g X\n", c0, c1);  
  fprintf(stdout, "covariance matrix:\n");  
  fprintf(stdout, "[ %g,  %g\n  %g,  %g]\n", cov00, cov01, cov01, cov11);   
  fprintf(stdout, "chisq = %g\n", chisq);   
  dh0 = 0.1 * h0;  
  fpout = fopen("fitout","w");  
  for (i = 0; i< 101; i++)  
    {  
      h0_fit = (h0-dh0) + i * 0.02 * dh0;  
      gsl_fit_linear_est(h0_fit, c0, c1, cov00, cov01, cov11, &y0, &y0_err);  
      fprintf(fpout, "%g  %g  %g\n", h0_fit, y0, y0_err);  
    }  
  
  fclose(fpout);  
  { 
    double h0_err_lo, h0_err_hi; 
    h0_err_lo = h0; 
    h0_err_hi = h0; 
    do{ 
      gsl_fit_linear_est(h0_err_hi, c0, c1, cov00, cov01, cov11, &y0, &y0_err);  
      h0_err_hi += 0.0001*h0; 
    } while (y0 - y0_err < 0.95); 
    fprintf(stdout, "deltah0hi = %11.7E\n", (h0_err_hi-h0)*1.0E-19); 
    do{ 
      gsl_fit_linear_est(h0_err_lo, c0, c1, cov00, cov01, cov11, &y0, &y0_err);  
      h0_err_lo -= 0.0001*h0; 
    } while (y0 + y0_err > 0.95); 
    fprintf(stdout, "deltah0lo = %11.7E\n", (h0 - h0_err_lo)*1.0E-19); 
    fprintf(stdout, "total 1-sigma percent error  %g\n", 100.0*(h0_err_hi - h0_err_lo)/h0); 
  } 
  
  free(x); 
  free(y); 
  free(w);

  return 0;
}

