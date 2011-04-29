/*
 * This source code is a product of Sun Microsystems, Inc. and is provided
 * for unrestricted use.  Users may copy or modify this source code without
 * charge.
 *
 * SUN SOURCE CODE IS PROVIDED AS IS WITH NO WARRANTIES OF ANY KIND INCLUDING
 * THE WARRANTIES OF DESIGN, MERCHANTIBILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE, OR ARISING FROM A COURSE OF DEALING, USAGE OR TRADE PRACTICE.
 *
 * Sun source code is provided with no support and without any obligation on
 * the part of Sun Microsystems, Inc. to assist in its use, correction,
 * modification or enhancement.
 *
 * SUN MICROSYSTEMS, INC. SHALL HAVE NO LIABILITY WITH RESPECT TO THE
 * INFRINGEMENT OF COPYRIGHTS, TRADE SECRETS OR ANY PATENTS BY THIS SOFTWARE
 * OR ANY PART THEREOF.
 *
 * In no event will Sun Microsystems, Inc. be liable for any lost revenue
 * or profits or other special, indirect and consequential damages, even if
 * Sun has been advised of the possibility of such damages.
 *
 * Sun Microsystems, Inc.
 * 2550 Garcia Avenue
 * Mountain View, California  94043
 */

#ifndef G72X_PRIVATE_H
#define G72X_PRIVATE_H


/*
** The following is the definition of the state structure used by the
** G.721/G.723 encoder and decoder to preserve their internal state
** between successive calls.  The meanings of the majority of the state 
** structure fields are explained in detail in the CCITT Recommendation 
** G.721.  The field names are essentially identical to variable names 
** in the bit level description of the coding algorithm included in this 
** Recommendation.
*/

typedef struct private_g72x
{	long  yl;	/* Locked or steady state step size multiplier. */
	short yu;	/* Unlocked or non-steady state step size multiplier. */
	short dms;	/* Short term energy estimate. */
	short dml;	/* Long term energy estimate. */
	short ap;	/* Linear weighting coefficient of 'yl' and 'yu'. */

	short a[2];	/* Coefficients of pole portion of prediction filter. */
	short b[6];	/* Coefficients of zero portion of prediction filter. */
	short pk[2];	/*
					** Signs of previous two samples of a partially
					** reconstructed signal.
					**/
	short dq[6];	/*
					** Previous 6 samples of the quantized difference
					** signal represented in an internal floating point
					** format.
					**/
	short sr[2];	/*
			 		** Previous 2 samples of the quantized difference
					** signal represented in an internal floating point
					** format.
					*/
	char td;	/* delayed tone detect, new in 1988 version */
	
	/*	The following struct members were added for libsndfile. The original 
	**	code worked by calling a set of functions on a sample by sample basis 
	**	which is slow on architectures like Intel x86. For libsndfile, this 
	**	was changed so that the encoding and decoding routines could work on
	**	a block of samples at a time to reduce the function call overhead.
	*/
#if defined(USE_LIBSND_ALL) || defined(USE_LIBSND_WRITE) 
	int		(*encoder) (int, struct private_g72x* state) ;
#endif
	int		(*decoder) (int, struct private_g72x* state) ;
	
	int		codec_bits ;
	int		byte_index, sample_index ;
	
} G72x_STATE ;



int	predictor_zero(G72x_STATE *state_ptr);

int	predictor_pole(G72x_STATE *state_ptr);

int	step_size(G72x_STATE *state_ptr);

int	quantize(int d, int	y, short *table, int size);

int	reconstruct(int sign, int dqln,	int y);

void update (int code_size, int y, int wi, int fi, int dq, int sr, int dqsez, G72x_STATE *state_ptr);

#if defined(USE_LIBSND_ALL) || defined(USE_LIBSND_WRITE) 
int g721_encoder	(int sample, G72x_STATE *state_ptr);
#endif
int g721_decoder	(int code, G72x_STATE *state_ptr);

#if defined(USE_LIBSND_ALL) || defined(USE_LIBSND_WRITE) 
int g723_16_encoder	(int sample, G72x_STATE *state_ptr);
#endif
int g723_16_decoder	(int code, G72x_STATE *state_ptr);

#if defined(USE_LIBSND_ALL) || defined(USE_LIBSND_WRITE) 
int g723_24_encoder	(int sample, G72x_STATE *state_ptr);
#endif
int g723_24_decoder	(int code, G72x_STATE *state_ptr);

#if defined(USE_LIBSND_ALL) || defined(USE_LIBSND_WRITE) 
int g723_40_encoder	(int sample, G72x_STATE *state_ptr);
#endif
int g723_40_decoder	(int code, G72x_STATE *state_ptr);



#endif /* G72X_PRIVATE_H */
