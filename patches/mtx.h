#ifndef _MTX_RECOMP_H_
#define _MTX_RECOMP_H_

typedef float Matrix[4][4];

struct FigureArrMtxSep {
    Matrix *view;
    Matrix *proj;
};

#define FIG_ARRAY_MAX 512

// if the ptrs are set, the figure has separate view and projection
// matrices. This will be used instead of the multiplied one on the CPU.

// 0 = view
// 1 = proj
extern Matrix gFigureMatrixArray[FIG_ARRAY_MAX][2]; // this is what is pointed to. Hope this doesnt overflow...

extern struct FigureArrMtxSep gFigureArrMtx[FIG_ARRAY_MAX];

#endif // _MTX_RECOMP_H_
