#include <gsl/gsl_complex.h>

#define RVEC(A) int A##n, double*A##p
#define RMAT(A) int A##r, int A##c, double* A##p
#define KRVEC(A) int A##n, const double*A##p
#define KRMAT(A) int A##r, int A##c, const double* A##p

#define CVEC(A) int A##n, gsl_complex*A##p
#define CMAT(A) int A##r, int A##c, gsl_complex* A##p
#define KCVEC(A) int A##n, const gsl_complex*A##p
#define KCMAT(A) int A##r, int A##c, const gsl_complex* A##p

#define FVEC(A) int A##n, float*A##p
#define FMAT(A) int A##r, int A##c, float* A##p
#define KFVEC(A) int A##n, const float*A##p
#define KFMAT(A) int A##r, int A##c, const float* A##p

#define QVEC(A) int A##n, gsl_complex_float*A##p
#define QMAT(A) int A##r, int A##c, gsl_complex_float* A##p
#define KQVEC(A) int A##n, const gsl_complex_float*A##p
#define KQMAT(A) int A##r, int A##c, const gsl_complex_float* A##p

#include <gsl/gsl_blas.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_fft_complex.h>
#include <gsl/gsl_integration.h>
#include <gsl/gsl_deriv.h>
#include <gsl/gsl_poly.h>
#include <gsl/gsl_multimin.h>
#include <gsl/gsl_multiroots.h>
#include <gsl/gsl_complex_math.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_multifit_nlin.h>
#include <string.h>
#include <stdio.h>

#define MACRO(B) do {B} while (0)
#define ERROR(CODE) MACRO(return CODE;)
#define REQUIRES(COND, CODE) MACRO(if(!(COND)) {ERROR(CODE);})
#define OK return 0;

#define MIN(A,B) ((A)<(B)?(A):(B))
#define MAX(A,B) ((A)>(B)?(A):(B))

#ifdef DBG
#define DEBUGMSG(M) printf("*** calling aux C function: %s\n",M);
#else
#define DEBUGMSG(M)
#endif

#define CHECK(RES,CODE) MACRO(if(RES) return CODE;)

#ifdef DBG
#define DEBUGMAT(MSG,X) printf(MSG" = \n"); gsl_matrix_fprintf(stdout,X,"%f"); printf("\n");
#else
#define DEBUGMAT(MSG,X)
#endif

#ifdef DBG
#define DEBUGVEC(MSG,X) printf(MSG" = \n"); gsl_vector_fprintf(stdout,X,"%f"); printf("\n");
#else
#define DEBUGVEC(MSG,X)
#endif

#define DVVIEW(A) gsl_vector_view A = gsl_vector_view_array(A##p,A##n)
#define DMVIEW(A) gsl_matrix_view A = gsl_matrix_view_array(A##p,A##r,A##c)
#define CVVIEW(A) gsl_vector_complex_view A = gsl_vector_complex_view_array((double*)A##p,A##n)
#define CMVIEW(A) gsl_matrix_complex_view A = gsl_matrix_complex_view_array((double*)A##p,A##r,A##c)
#define KDVVIEW(A) gsl_vector_const_view A = gsl_vector_const_view_array(A##p,A##n)
#define KDMVIEW(A) gsl_matrix_const_view A = gsl_matrix_const_view_array(A##p,A##r,A##c)
#define KCVVIEW(A) gsl_vector_complex_const_view A = gsl_vector_complex_const_view_array((double*)A##p,A##n)
#define KCMVIEW(A) gsl_matrix_complex_const_view A = gsl_matrix_complex_const_view_array((double*)A##p,A##r,A##c)

#define FVVIEW(A) gsl_vector_float_view A = gsl_vector_float_view_array(A##p,A##n)
#define FMVIEW(A) gsl_matrix_float_view A = gsl_matrix_float_view_array(A##p,A##r,A##c)
#define QVVIEW(A) gsl_vector_complex_float_view A = gsl_vector_float_complex_view_array((float*)A##p,A##n)
#define QMVIEW(A) gsl_matrix_complex_float_view A = gsl_matrix_float_complex_view_array((float*)A##p,A##r,A##c)
#define KFVVIEW(A) gsl_vector_float_const_view A = gsl_vector_float_const_view_array(A##p,A##n)
#define KFMVIEW(A) gsl_matrix_float_const_view A = gsl_matrix_float_const_view_array(A##p,A##r,A##c)
#define KQVVIEW(A) gsl_vector_complex_float_const_view A = gsl_vector_complex_float_const_view_array((float*)A##p,A##n)
#define KQMVIEW(A) gsl_matrix_complex_float_const_view A = gsl_matrix_complex_float_const_view_array((float*)A##p,A##r,A##c)

#define V(a) (&a.vector)
#define M(a) (&a.matrix)

#define GCVEC(A) int A##n, gsl_complex*A##p
#define KGCVEC(A) int A##n, const gsl_complex*A##p

#define GQVEC(A) int A##n, gsl_complex_float*A##p
#define KGQVEC(A) int A##n, const gsl_complex_float*A##p

#define BAD_SIZE 2000
#define BAD_CODE 2001
#define MEM      2002
#define BAD_FILE 2003


void no_abort_on_error() {
    gsl_set_error_handler_off();
}




int fft(int code, KCVEC(X), CVEC(R)) {
    REQUIRES(Xn == Rn,BAD_SIZE);
    DEBUGMSG("fft");
    int s = Xn;
    gsl_fft_complex_wavetable * wavetable = gsl_fft_complex_wavetable_alloc (s);
    gsl_fft_complex_workspace * workspace = gsl_fft_complex_workspace_alloc (s);
    gsl_vector_const_view X = gsl_vector_const_view_array((double*)Xp, 2*Xn);
    gsl_vector_view R = gsl_vector_view_array((double*)Rp, 2*Rn);
    gsl_blas_dcopy(&X.vector,&R.vector);
    if(code==0) {
        gsl_fft_complex_forward ((double*)Rp, 1, s, wavetable, workspace);
    } else {
        gsl_fft_complex_inverse ((double*)Rp, 1, s, wavetable, workspace);
    }
    gsl_fft_complex_wavetable_free (wavetable);
    gsl_fft_complex_workspace_free (workspace);
    OK
}


int deriv(int code, double f(double, void*), double x, double h, double * result, double * abserr)
{
    gsl_function F;
    F.function = f;
    F.params = 0;

    if(code==0) return gsl_deriv_central (&F, x, h, result, abserr);

    if(code==1) return gsl_deriv_forward (&F, x, h, result, abserr);

    if(code==2) return gsl_deriv_backward (&F, x, h, result, abserr);

    return 0;
}


int integrate_qng(double f(double, void*), double a, double b, double prec,
                   double *result, double*error) {
    DEBUGMSG("integrate_qng");
    gsl_function F;
    F.function = f;
    F.params = NULL;
    size_t neval;
    int res = gsl_integration_qng (&F, a,b, 0, prec, result, error, &neval); 
    CHECK(res,res);
    OK
}

int integrate_qags(double f(double,void*), double a, double b, double prec, int w, 
               double *result, double* error) {
    DEBUGMSG("integrate_qags");
    gsl_integration_workspace * wk = gsl_integration_workspace_alloc (w);
    gsl_function F;
    F.function = f;
    F.params = NULL;
    int res = gsl_integration_qags (&F, a,b, 0, prec, w,wk, result, error); 
    CHECK(res,res);
    gsl_integration_workspace_free (wk); 
    OK
}

int integrate_qagi(double f(double,void*), double prec, int w, 
               double *result, double* error) {
    DEBUGMSG("integrate_qagi");
    gsl_integration_workspace * wk = gsl_integration_workspace_alloc (w);
    gsl_function F;
    F.function = f;
    F.params = NULL;
    int res = gsl_integration_qagi (&F, 0, prec, w,wk, result, error); 
    CHECK(res,res);
    gsl_integration_workspace_free (wk); 
    OK
}


int integrate_qagiu(double f(double,void*), double a, double prec, int w, 
               double *result, double* error) {
    DEBUGMSG("integrate_qagiu");
    gsl_integration_workspace * wk = gsl_integration_workspace_alloc (w);
    gsl_function F;
    F.function = f;
    F.params = NULL;
    int res = gsl_integration_qagiu (&F, a, 0, prec, w,wk, result, error); 
    CHECK(res,res);
    gsl_integration_workspace_free (wk); 
    OK
}


int integrate_qagil(double f(double,void*), double b, double prec, int w, 
               double *result, double* error) {
    DEBUGMSG("integrate_qagil");
    gsl_integration_workspace * wk = gsl_integration_workspace_alloc (w);
    gsl_function F;
    F.function = f;
    F.params = NULL;
    int res = gsl_integration_qagil (&F, b, 0, prec, w,wk, result, error); 
    CHECK(res,res);
    gsl_integration_workspace_free (wk); 
    OK
}


int polySolve(KRVEC(a), CVEC(z)) {
    DEBUGMSG("polySolve");
    REQUIRES(an>1,BAD_SIZE);
    gsl_poly_complex_workspace * w = gsl_poly_complex_workspace_alloc (an);
    int res = gsl_poly_complex_solve ((double*)ap, an, w, (double*)zp);
    CHECK(res,res);
    gsl_poly_complex_workspace_free (w);
    OK;
}


//---------------------------------------------------------------

typedef double Trawfun(int, double*);

double only_f_aux_min(const gsl_vector*x, void *pars) {
    Trawfun * f = (Trawfun*) pars;  
    double* p = (double*)calloc(x->size,sizeof(double));
    int k;
    for(k=0;k<x->size;k++) {
        p[k] = gsl_vector_get(x,k);
    }  
    double res = f(x->size,p);
    free(p);
    return res;
}

// this version returns info about intermediate steps
int minimize(int method, double f(int, double*), double tolsize, int maxit, 
                 KRVEC(xi), KRVEC(sz), RMAT(sol)) {
    REQUIRES(xin==szn && solr == maxit && solc == 3+xin,BAD_SIZE);
    DEBUGMSG("minimizeList (nmsimplex)");
    gsl_multimin_function my_func;
    // extract function from pars
    my_func.f = only_f_aux_min;
    my_func.n = xin; 
    my_func.params = f;
    size_t iter = 0;
    int status;
    double size;
    const gsl_multimin_fminimizer_type *T;
    gsl_multimin_fminimizer *s = NULL;
    // Initial vertex size vector 
    KDVVIEW(sz);
    // Starting point
    KDVVIEW(xi);
    // Minimizer nmsimplex, without derivatives
    switch(method) {
        case 0 : {T = gsl_multimin_fminimizer_nmsimplex; break; }
#ifdef GSL110
        case 1 : {T = gsl_multimin_fminimizer_nmsimplex; break; }
#else
        case 1 : {T = gsl_multimin_fminimizer_nmsimplex2; break; }
#endif
        default: ERROR(BAD_CODE);
    }
    s = gsl_multimin_fminimizer_alloc (T, my_func.n);
    gsl_multimin_fminimizer_set (s, &my_func, V(xi), V(sz));
    do {
        status = gsl_multimin_fminimizer_iterate (s);
        size = gsl_multimin_fminimizer_size (s);

        solp[iter*solc+0] = iter+1;
        solp[iter*solc+1] = s->fval;
        solp[iter*solc+2] = size;

        int k;
        for(k=0;k<xin;k++) {
            solp[iter*solc+k+3] = gsl_vector_get(s->x,k);
        }
        iter++;
        if (status) break;
        status = gsl_multimin_test_size (size, tolsize);
    } while (status == GSL_CONTINUE && iter <= maxit);
    int i,j;
    for (i=iter; i<solr; i++) {
        solp[i*solc+0] = iter;
        for(j=1;j<solc;j++) {
            solp[i*solc+j]=0.;
        }
    }
    gsl_multimin_fminimizer_free(s);
    OK
}

// working with the gradient

typedef struct {double (*f)(int, double*); int (*df)(int, double*, int, double*);} Tfdf;

double f_aux_min(const gsl_vector*x, void *pars) {
    Tfdf * fdf = ((Tfdf*) pars);
    double* p = (double*)calloc(x->size,sizeof(double));
    int k;
    for(k=0;k<x->size;k++) {
        p[k] = gsl_vector_get(x,k);
    }
    double res = fdf->f(x->size,p);
    free(p);
    return res;
}


void df_aux_min(const gsl_vector * x, void * pars, gsl_vector * g) {
    Tfdf * fdf = ((Tfdf*) pars);
    double* p = (double*)calloc(x->size,sizeof(double));
    double* q = (double*)calloc(g->size,sizeof(double));
    int k;
    for(k=0;k<x->size;k++) {
        p[k] = gsl_vector_get(x,k);
    }

    fdf->df(x->size,p,g->size,q);

    for(k=0;k<x->size;k++) {
        gsl_vector_set(g,k,q[k]);
    }
    free(p);
    free(q);
}

void fdf_aux_min(const gsl_vector * x, void * pars, double * f, gsl_vector * g) {
    *f = f_aux_min(x,pars);
    df_aux_min(x,pars,g);
}


int minimizeD(int method, double f(int, double*), int df(int, double*, int, double*),
              double initstep, double minimpar, double tolgrad, int maxit, 
              KRVEC(xi), RMAT(sol)) {
    REQUIRES(solr == maxit && solc == 2+xin,BAD_SIZE);
    DEBUGMSG("minimizeWithDeriv (conjugate_fr)");
    gsl_multimin_function_fdf my_func;
    // extract function from pars
    my_func.f = f_aux_min;
    my_func.df = df_aux_min;
    my_func.fdf = fdf_aux_min;
    my_func.n = xin; 
    Tfdf stfdf;
    stfdf.f = f;
    stfdf.df = df;
    my_func.params = &stfdf;
    size_t iter = 0;
    int status;
    const gsl_multimin_fdfminimizer_type *T;
    gsl_multimin_fdfminimizer *s = NULL;
    // Starting point
    KDVVIEW(xi);
    // conjugate gradient fr
    switch(method) {
        case 0 : {T = gsl_multimin_fdfminimizer_conjugate_fr; break; }
        case 1 : {T = gsl_multimin_fdfminimizer_conjugate_pr; break; }
        case 2 : {T = gsl_multimin_fdfminimizer_vector_bfgs; break; }
        case 3 : {T = gsl_multimin_fdfminimizer_vector_bfgs2; break; }
        case 4 : {T = gsl_multimin_fdfminimizer_steepest_descent; break; }
        default: ERROR(BAD_CODE);
    }
    s = gsl_multimin_fdfminimizer_alloc (T, my_func.n);
    gsl_multimin_fdfminimizer_set (s, &my_func, V(xi), initstep, minimpar);
    do {
        status = gsl_multimin_fdfminimizer_iterate (s);
        solp[iter*solc+0] = iter+1;
        solp[iter*solc+1] = s->f;
        int k;
        for(k=0;k<xin;k++) {
            solp[iter*solc+k+2] = gsl_vector_get(s->x,k);
        }
        iter++;
        if (status) break;
        status = gsl_multimin_test_gradient (s->gradient, tolgrad);
    } while (status == GSL_CONTINUE && iter <= maxit);
    int i,j;
    for (i=iter; i<solr; i++) {
        solp[i*solc+0] = iter;
        for(j=1;j<solc;j++) {
            solp[i*solc+j]=0.;
        }
    }
    gsl_multimin_fdfminimizer_free(s);
    OK
}

//---------------------------------------------------------------

typedef void TrawfunV(int, double*, int, double*);

int only_f_aux_root(const gsl_vector*x, void *pars, gsl_vector*y) {
    TrawfunV * f = (TrawfunV*) pars;
    double* p = (double*)calloc(x->size,sizeof(double));
    double* q = (double*)calloc(y->size,sizeof(double));
    int k;
    for(k=0;k<x->size;k++) {
        p[k] = gsl_vector_get(x,k);
    }
    f(x->size,p,y->size,q);
    for(k=0;k<y->size;k++) {
        gsl_vector_set(y,k,q[k]);
    }
    free(p);
    free(q);
    return 0; //hmmm
}

int root(int method, void f(int, double*, int, double*),
         double epsabs, int maxit,
         KRVEC(xi), RMAT(sol)) {
    REQUIRES(solr == maxit && solc == 1+2*xin,BAD_SIZE);
    DEBUGMSG("root_only_f");
    gsl_multiroot_function my_func;
    // extract function from pars
    my_func.f = only_f_aux_root;
    my_func.n = xin;
    my_func.params = f;
    size_t iter = 0;
    int status;
    const gsl_multiroot_fsolver_type *T;
    gsl_multiroot_fsolver *s;
    // Starting point
    KDVVIEW(xi);
    switch(method) {
        case 0 : {T = gsl_multiroot_fsolver_hybrids;; break; }
        case 1 : {T = gsl_multiroot_fsolver_hybrid; break; }
        case 2 : {T = gsl_multiroot_fsolver_dnewton; break; }
        case 3 : {T = gsl_multiroot_fsolver_broyden; break; }
        default: ERROR(BAD_CODE);
    }
    s = gsl_multiroot_fsolver_alloc (T, my_func.n);
    gsl_multiroot_fsolver_set (s, &my_func, V(xi));

    do {
           status = gsl_multiroot_fsolver_iterate (s);

           solp[iter*solc+0] = iter+1;

           int k;
           for(k=0;k<xin;k++) {
               solp[iter*solc+k+1] = gsl_vector_get(s->x,k);
           }
           for(k=xin;k<2*xin;k++) {
               solp[iter*solc+k+1] = gsl_vector_get(s->f,k-xin);
           }

           iter++;
           if (status)   /* check if solver is stuck */
             break;

           status =
             gsl_multiroot_test_residual (s->f, epsabs);
        }
        while (status == GSL_CONTINUE && iter <= maxit);

    int i,j;
    for (i=iter; i<solr; i++) {
        solp[i*solc+0] = iter;
        for(j=1;j<solc;j++) {
            solp[i*solc+j]=0.;
        }
    }
    gsl_multiroot_fsolver_free(s);
    OK
}

// working with the jacobian

typedef struct {int (*f)(int, double*, int, double *);
                int (*jf)(int, double*, int, int, double*);} Tfjf;

int f_aux(const gsl_vector*x, void *pars, gsl_vector*y) {
    Tfjf * fjf = ((Tfjf*) pars);
    double* p = (double*)calloc(x->size,sizeof(double));
    double* q = (double*)calloc(y->size,sizeof(double));
    int k;
    for(k=0;k<x->size;k++) {
        p[k] = gsl_vector_get(x,k);
    }
    (fjf->f)(x->size,p,y->size,q);
    for(k=0;k<y->size;k++) {
        gsl_vector_set(y,k,q[k]);
    }
    free(p);
    free(q);
    return 0;
}

int jf_aux(const gsl_vector * x, void * pars, gsl_matrix * jac) {
    Tfjf * fjf = ((Tfjf*) pars);
    double* p = (double*)calloc(x->size,sizeof(double));
    double* q = (double*)calloc((jac->size1)*(jac->size2),sizeof(double));
    int i,j,k;
    for(k=0;k<x->size;k++) {
        p[k] = gsl_vector_get(x,k);
    }

    (fjf->jf)(x->size,p,jac->size1,jac->size2,q);

    k=0;
    for(i=0;i<jac->size1;i++) {
        for(j=0;j<jac->size2;j++){
            gsl_matrix_set(jac,i,j,q[k++]);
        }
    }
    free(p);
    free(q);
    return 0;
}

int fjf_aux(const gsl_vector * x, void * pars, gsl_vector * f, gsl_matrix * g) {
    f_aux(x,pars,f);
    jf_aux(x,pars,g);
    return 0;
}

int rootj(int method, int f(int, double*, int, double*),
                      int jac(int, double*, int, int, double*),
         double epsabs, int maxit,
         KRVEC(xi), RMAT(sol)) {
    REQUIRES(solr == maxit && solc == 1+2*xin,BAD_SIZE);
    DEBUGMSG("root_fjf");
    gsl_multiroot_function_fdf my_func;
    // extract function from pars
    my_func.f = f_aux;
    my_func.df = jf_aux;
    my_func.fdf = fjf_aux;
    my_func.n = xin;
    Tfjf stfjf;
    stfjf.f = f;
    stfjf.jf = jac;
    my_func.params = &stfjf;
    size_t iter = 0;
    int status;
    const gsl_multiroot_fdfsolver_type *T;
    gsl_multiroot_fdfsolver *s;
    // Starting point
    KDVVIEW(xi);
    switch(method) {
        case 0 : {T = gsl_multiroot_fdfsolver_hybridsj;; break; }
        case 1 : {T = gsl_multiroot_fdfsolver_hybridj; break; }
        case 2 : {T = gsl_multiroot_fdfsolver_newton; break; }
        case 3 : {T = gsl_multiroot_fdfsolver_gnewton; break; }
        default: ERROR(BAD_CODE);
    }
    s = gsl_multiroot_fdfsolver_alloc (T, my_func.n);

    gsl_multiroot_fdfsolver_set (s, &my_func, V(xi));

    do {
           status = gsl_multiroot_fdfsolver_iterate (s);

           solp[iter*solc+0] = iter+1;

           int k;
           for(k=0;k<xin;k++) {
               solp[iter*solc+k+1] = gsl_vector_get(s->x,k);
           }
           for(k=xin;k<2*xin;k++) {
               solp[iter*solc+k+1] = gsl_vector_get(s->f,k-xin);
           }

           iter++;
           if (status)   /* check if solver is stuck */
             break;

           status =
             gsl_multiroot_test_residual (s->f, epsabs);
        }
        while (status == GSL_CONTINUE && iter <= maxit);

    int i,j;
    for (i=iter; i<solr; i++) {
        solp[i*solc+0] = iter;
        for(j=1;j<solc;j++) {
            solp[i*solc+j]=0.;
        }
    }
    gsl_multiroot_fdfsolver_free(s);
    OK
}

//-------------- non linear least squares fitting -------------------

int nlfit(int method, int f(int, double*, int, double*),
                      int jac(int, double*, int, int, double*),
         double epsabs, double epsrel, int maxit, int p,
         KRVEC(xi), RMAT(sol)) {
    REQUIRES(solr == maxit && solc == 2+xin,BAD_SIZE);
    DEBUGMSG("nlfit");
    const gsl_multifit_fdfsolver_type *T;
    gsl_multifit_fdfsolver *s;
    gsl_multifit_function_fdf my_f;
    // extract function from pars
    my_f.f = f_aux;
    my_f.df = jf_aux;
    my_f.fdf = fjf_aux;
    my_f.n = p;
    my_f.p = xin;  // !!!!
    Tfjf stfjf;
    stfjf.f = f;
    stfjf.jf = jac;
    my_f.params = &stfjf;
    size_t iter = 0;
    int status;

    KDVVIEW(xi);
    //DMVIEW(cov);

    switch(method) {
        case 0 : { T = gsl_multifit_fdfsolver_lmsder; break; }
        case 1 : { T = gsl_multifit_fdfsolver_lmder; break; }
        default: ERROR(BAD_CODE);
    }

    s = gsl_multifit_fdfsolver_alloc (T, my_f.n, my_f.p);
    gsl_multifit_fdfsolver_set (s, &my_f, V(xi));

    do {   status = gsl_multifit_fdfsolver_iterate (s);

           solp[iter*solc+0] = iter+1;
           solp[iter*solc+1] = gsl_blas_dnrm2 (s->f);

           int k;
           for(k=0;k<xin;k++) {
               solp[iter*solc+k+2] = gsl_vector_get(s->x,k);
           }

           iter++;
           if (status)   /* check if solver is stuck */
             break;

           status = gsl_multifit_test_delta (s->dx, s->x, epsabs, epsrel);
        }
        while (status == GSL_CONTINUE && iter <= maxit);

    int i,j;
    for (i=iter; i<solr; i++) {
        solp[i*solc+0] = iter;
        for(j=1;j<solc;j++) {
            solp[i*solc+j]=0.;
        }
    }

    //gsl_multifit_covar (s->J, 0.0, M(cov));

    gsl_multifit_fdfsolver_free (s);
    OK
}

//////////////////////////////////////////////////////

#include "gsl-ode.c"

//////////////////////////////////////////////////////