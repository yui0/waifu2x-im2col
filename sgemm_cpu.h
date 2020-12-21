/* public domain Simple, Minimalistic, Fast GEMM library
 *	©2020 Yuichiro Nakada
 *
 * Basic usage:
 *	sgemm_cpu('N', 'N', M, N, K, A, B, C);
 * */

inline void sgemm_cpu(char ta, char tb, int M, int N, int K, float *A, float *B, float *C)
{
	#pragma omp parallel for
	// CNN
/*	for (int m=0; m<M; m++) {
		for (int n=0; n<N; n++) {
			float acc = 0.0f;
			for (int k=0; k<K; k++) {
				acc += A[k*M + m] * B[n*K + k];
			}
			C[n*M + m] = acc;
		}
	}*/
	// RNN
/*	for (int m=0; m<M; m++) {
		for (int n=0; n<N; n++) {
			float acc = 0.0f;
			for (int k=0; k<K; k++) {
				acc += A[k + m*K] * B[n + k*N];
			}
			C[n + m*N] = acc;
		}
	}*/
/*	for (int m=0; m<M; m++) { // slow
		for (int n=0; n<N; n++) {
//			float acc = 0.0f;
			for (int k=0; k<K; k++) {
//				acc += A[k + m*M] * B[n + k*N];
				C[m*N+n] += A[k + m*M] * B[n + k*N];
			}
//			C[n + m*N] = acc;
		}
	}*/
	memset(C, 0, M*N*sizeof(float));
	for (int m=0; m<M; ++m) { // fast
		for (int k=0; k<K; ++k) {
			//register float A_PART = ALPHA*A[i*lda+k];
			for (int n=0; n<N; ++n) {
//				C[i*ldc+j] += A_PART*B[k*ldb+j];
//				C[m*N+n] += A[m*M+k] * B[k*N+n];
				C[m*N+n] += A[m*K+k] * B[k*N+n];
			}
		}
	}
	// RNT
/*	for (int m=0; m<M; m++) {
		for (int n=0; n<N; n++) {
			float acc = 0.0f;
			for (int k=0; k<K; k++) {
				acc += A[k + m*K] * B[n*K + k];
			}
			C[n + m*N] = acc;
		}
	}*/
}

static inline void im2col(const float *im, const int channels,
	const int height, const int width, const int kernel_h, const int kernel_w,
	const int pad_h, const int pad_w, const int stride_h, const int stride_w, float *col)
{
	int height_col = (height + 2 * pad_h - kernel_h) / stride_h + 1;
	int width_col = (width + 2 * pad_w - kernel_w) / stride_w + 1;
	int channels_col = channels * kernel_h * kernel_w;

	for (int c=0; c<channels_col; c++) {
		int w_offset = c % kernel_w;
		int h_offset = (c / kernel_w) % kernel_h;
		int c_im = c / kernel_h / kernel_w;
		for (int h=0; h<height_col; h++) {
			for (int w=0; w<width_col; w++) {
				int h_pad = h * stride_h - pad_h + h_offset;
				int w_pad = w * stride_w - pad_w + w_offset;
				if (h_pad >= 0 && h_pad < height && w_pad >= 0 && w_pad < width)
					col[(c * height_col + h) * width_col + w] =
						im[(c_im * height + h_pad) * width + w_pad];
				else
					col[(c * height_col + h) * width_col + w] = 0;
			}
		}
	}
}
//#define max(a,b) ((a) > (b) ? (a) : (b))
//#define min(a,b) ((a) < (b) ? (a) : (b))
float workspace[256*256*128*64];
static inline void cpu_convolution_LReLU(float *inputs, int ich, int w, int h, float *weights, int k, int pad, int stride, float *outputs, int ch, float *bias)
{
	// im2col(pix, 3, h, w, 4, 4, 2, 2, 1, 1, workspace);
	im2col(inputs, ich, h, w, k, k, pad, pad, stride, stride, workspace);
	int hcol = (h + 2 * pad - k) / stride + 1;
	int wcol = (w + 2 * pad - k) / stride + 1;

	// gemm('N', 'N', ch, wcol*hcol, k*k*ich, magic_kernel, workspace, pix);
	// https://petewarden.com/2015/04/20/why-gemm-is-at-the-heart-of-deep-learning/
	sgemm_cpu('N', 'N', ch, wcol*hcol/* *batch */, k*k*ich, weights, workspace, outputs);

	float *p = outputs;
	for (int i=0; i<ch; i++) {
		for (int n=0; n<wcol*hcol; n++) {
			*p += bias[i];
			*p = *p>0 ? (*p) : (*p)*0.1;
			p++;
//			float a = *p + bias[i];
//			*p++ = max(a, 0.0) + min(a, 0.0) * 0.1;
		}
	}
}

