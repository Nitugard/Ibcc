/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#include "HandmadeMath.h"


COVERAGE(HMM_Power, 2)
float HMM_PREFIX(Power)(float Base, int Exponent)
{
    ASSERT_COVERED(HMM_Power);

    float Result = 1.0f;
    float Mul = Exponent < 0 ? 1.f / Base : Base;
    int X = Exponent < 0 ? -Exponent : Exponent;
    while (X)
    {
        if (X & 1)
        {
            ASSERT_COVERED(HMM_Power);

            Result *= Mul;
        }

        Mul *= Mul;
        X >>= 1;
    }

    return (Result);
}

#ifndef HANDMADE_MATH__USE_SSE
COVERAGE(HMM_Transpose, 1)
hmm_mat4 HMM_PREFIX(Transpose)(hmm_mat4 Matrix)
{
    ASSERT_COVERED(HMM_Transpose);

    hmm_mat4 Result;

    int Columns;
    for(Columns = 0; Columns < 4; ++Columns)
    {
        int Rows;
        for(Rows = 0; Rows < 4; ++Rows)
        {
            Result.Elements[Rows][Columns] = Matrix.Elements[Columns][Rows];
        }
    }

    return (Result);
}
#endif

#ifndef HANDMADE_MATH__USE_SSE
COVERAGE(HMM_AddMat4, 1)
hmm_mat4 HMM_PREFIX(AddMat4)(hmm_mat4 Left, hmm_mat4 Right)
{
    ASSERT_COVERED(HMM_AddMat4);

    hmm_mat4 Result;

    int Columns;
    for(Columns = 0; Columns < 4; ++Columns)
    {
        int Rows;
        for(Rows = 0; Rows < 4; ++Rows)
        {
            Result.Elements[Columns][Rows] = Left.Elements[Columns][Rows] + Right.Elements[Columns][Rows];
        }
    }

    return (Result);
}
#endif

#ifndef HANDMADE_MATH__USE_SSE
COVERAGE(HMM_SubtractMat4, 1)
hmm_mat4 HMM_PREFIX(SubtractMat4)(hmm_mat4 Left, hmm_mat4 Right)
{
    ASSERT_COVERED(HMM_SubtractMat4);

    hmm_mat4 Result;

    int Columns;
    for(Columns = 0; Columns < 4; ++Columns)
    {
        int Rows;
        for(Rows = 0; Rows < 4; ++Rows)
        {
            Result.Elements[Columns][Rows] = Left.Elements[Columns][Rows] - Right.Elements[Columns][Rows];
        }
    }

    return (Result);
}
#endif

COVERAGE(HMM_MultiplyMat4, 1)
hmm_mat4 HMM_PREFIX(MultiplyMat4)(hmm_mat4 Left, hmm_mat4 Right)
{
    ASSERT_COVERED(HMM_MultiplyMat4);

    hmm_mat4 Result;

#ifdef HANDMADE_MATH__USE_SSE
    Result.Columns[0] = HMM_PREFIX(LinearCombineSSE)(Right.Columns[0], Left);
    Result.Columns[1] = HMM_PREFIX(LinearCombineSSE)(Right.Columns[1], Left);
    Result.Columns[2] = HMM_PREFIX(LinearCombineSSE)(Right.Columns[2], Left);
    Result.Columns[3] = HMM_PREFIX(LinearCombineSSE)(Right.Columns[3], Left);
#else
    int Columns;
    for(Columns = 0; Columns < 4; ++Columns)
    {
        int Rows;
        for(Rows = 0; Rows < 4; ++Rows)
        {
            float Sum = 0;
            int CurrentMatrice;
            for(CurrentMatrice = 0; CurrentMatrice < 4; ++CurrentMatrice)
            {
                Sum += Left.Elements[CurrentMatrice][Rows] * Right.Elements[Columns][CurrentMatrice];
            }

            Result.Elements[Columns][Rows] = Sum;
        }
    }
#endif

    return (Result);
}

#ifndef HANDMADE_MATH__USE_SSE
COVERAGE(HMM_MultiplyMat4f, 1)
hmm_mat4 HMM_PREFIX(MultiplyMat4f)(hmm_mat4 Matrix, float Scalar)
{
    ASSERT_COVERED(HMM_MultiplyMat4f);

    hmm_mat4 Result;

    int Columns;
    for(Columns = 0; Columns < 4; ++Columns)
    {
        int Rows;
        for(Rows = 0; Rows < 4; ++Rows)
        {
            Result.Elements[Columns][Rows] = Matrix.Elements[Columns][Rows] * Scalar;
        }
    }

    return (Result);
}
#endif

COVERAGE(HMM_MultiplyMat4ByVec4, 1)
hmm_vec4 HMM_PREFIX(MultiplyMat4ByVec4)(hmm_mat4 Matrix, hmm_vec4 Vector)
{
    ASSERT_COVERED(HMM_MultiplyMat4ByVec4);

    hmm_vec4 Result;

#ifdef HANDMADE_MATH__USE_SSE
	Result.InternalElementsSSE = HMM_PREFIX(LinearCombineSSE)(Vector.InternalElementsSSE, Matrix);
#else
    int Columns, Rows;
    for(Rows = 0; Rows < 4; ++Rows)
    {
        float Sum = 0;
        for(Columns = 0; Columns < 4; ++Columns)
        {
            Sum += Matrix.Elements[Columns][Rows] * Vector.Elements[Columns];
        }

        Result.Elements[Rows] = Sum;
    }
#endif

    return (Result);
}

#ifndef HANDMADE_MATH__USE_SSE
COVERAGE(HMM_DivideMat4f, 1);
hmm_mat4 HMM_PREFIX(DivideMat4f)(hmm_mat4 Matrix, float Scalar)
{
    ASSERT_COVERED(HMM_DivideMat4f);

    hmm_mat4 Result;

    int Columns;
    for(Columns = 0; Columns < 4; ++Columns)
    {
        int Rows;
        for(Rows = 0; Rows < 4; ++Rows)
        {
            Result.Elements[Columns][Rows] = Matrix.Elements[Columns][Rows] / Scalar;
        }
    }

    return (Result);
}
#endif

COVERAGE(HMM_Rotate, 1)
hmm_mat4 HMM_PREFIX(Rotate)(float Angle, hmm_vec3 Axis)
{
    ASSERT_COVERED(HMM_Rotate);

    hmm_mat4 Result = HMM_PREFIX(Mat4d)(1.0f);

    Axis = HMM_PREFIX(NormalizeVec3)(Axis);

    float SinTheta = HMM_PREFIX(SinF)(HMM_PREFIX(ToRadians)(Angle));
    float CosTheta = HMM_PREFIX(CosF)(HMM_PREFIX(ToRadians)(Angle));
    float CosValue = 1.0f - CosTheta;

    Result.Elements[0][0] = (Axis.X * Axis.X * CosValue) + CosTheta;
    Result.Elements[0][1] = (Axis.X * Axis.Y * CosValue) + (Axis.Z * SinTheta);
    Result.Elements[0][2] = (Axis.X * Axis.Z * CosValue) - (Axis.Y * SinTheta);

    Result.Elements[1][0] = (Axis.Y * Axis.X * CosValue) - (Axis.Z * SinTheta);
    Result.Elements[1][1] = (Axis.Y * Axis.Y * CosValue) + CosTheta;
    Result.Elements[1][2] = (Axis.Y * Axis.Z * CosValue) + (Axis.X * SinTheta);

    Result.Elements[2][0] = (Axis.Z * Axis.X * CosValue) + (Axis.Y * SinTheta);
    Result.Elements[2][1] = (Axis.Z * Axis.Y * CosValue) - (Axis.X * SinTheta);
    Result.Elements[2][2] = (Axis.Z * Axis.Z * CosValue) + CosTheta;

    return (Result);
}

COVERAGE(HMM_LookAt, 1)
hmm_mat4 HMM_PREFIX(LookAt)(hmm_vec3 Eye, hmm_vec3 Center, hmm_vec3 Up)
{
    ASSERT_COVERED(HMM_LookAt);

    hmm_mat4 Result;

    hmm_vec3 F = HMM_PREFIX(NormalizeVec3)(HMM_PREFIX(SubtractVec3)(Center, Eye));
    hmm_vec3 S = HMM_PREFIX(NormalizeVec3)(HMM_PREFIX(Cross)(F, Up));
    hmm_vec3 U = HMM_PREFIX(Cross)(S, F);

    Result.Elements[0][0] = S.X;
    Result.Elements[0][1] = U.X;
    Result.Elements[0][2] = -F.X;
    Result.Elements[0][3] = 0.0f;

    Result.Elements[1][0] = S.Y;
    Result.Elements[1][1] = U.Y;
    Result.Elements[1][2] = -F.Y;
    Result.Elements[1][3] = 0.0f;

    Result.Elements[2][0] = S.Z;
    Result.Elements[2][1] = U.Z;
    Result.Elements[2][2] = -F.Z;
    Result.Elements[2][3] = 0.0f;

    Result.Elements[3][0] = -HMM_PREFIX(DotVec3)(S, Eye);
    Result.Elements[3][1] = -HMM_PREFIX(DotVec3)(U, Eye);
    Result.Elements[3][2] = HMM_PREFIX(DotVec3)(F, Eye);
    Result.Elements[3][3] = 1.0f;

    return (Result);
}

COVERAGE(HMM_InverseQuaternion, 1)
hmm_quaternion HMM_PREFIX(InverseQuaternion)(hmm_quaternion Left)
{
    ASSERT_COVERED(HMM_InverseQuaternion);

    hmm_quaternion Conjugate;
    hmm_quaternion Result;
    float Norm = 0;
    float NormSquared = 0;

    Conjugate.X = -Left.X;
    Conjugate.Y = -Left.Y;
    Conjugate.Z = -Left.Z;
    Conjugate.W = Left.W;

    Norm = HMM_PREFIX(SquareRootF)(HMM_PREFIX(DotQuaternion)(Left, Left));
    NormSquared = Norm * Norm;

    Result = HMM_PREFIX(DivideQuaternionF)(Conjugate, NormSquared);

    return (Result);
}

COVERAGE(HMM_Slerp, 1)
hmm_quaternion HMM_PREFIX(Slerp)(hmm_quaternion Left, float Time, hmm_quaternion Right)
{
    ASSERT_COVERED(HMM_Slerp);

    hmm_quaternion Result;
    hmm_quaternion QuaternionLeft;
    hmm_quaternion QuaternionRight;

    float Cos_Theta = HMM_PREFIX(DotQuaternion)(Left, Right);
    float Angle = HMM_PREFIX(ACosF)(Cos_Theta);

    float S1 = HMM_PREFIX(SinF)((1.0f - Time) * Angle);
    float S2 = HMM_PREFIX(SinF)(Time * Angle);
    float Is = 1.0f / HMM_PREFIX(SinF)(Angle);

    QuaternionLeft = HMM_PREFIX(MultiplyQuaternionF)(Left, S1);
    QuaternionRight = HMM_PREFIX(MultiplyQuaternionF)(Right, S2);

    Result = HMM_PREFIX(AddQuaternion)(QuaternionLeft, QuaternionRight);
    Result = HMM_PREFIX(MultiplyQuaternionF)(Result, Is);

    return (Result);
}

COVERAGE(HMM_QuaternionToMat4, 1)
hmm_mat4 HMM_PREFIX(QuaternionToMat4)(hmm_quaternion Left)
{
    ASSERT_COVERED(HMM_QuaternionToMat4);

    hmm_mat4 Result;

    hmm_quaternion NormalizedQuaternion = HMM_PREFIX(NormalizeQuaternion)(Left);

    float XX, YY, ZZ,
          XY, XZ, YZ,
          WX, WY, WZ;

    XX = NormalizedQuaternion.X * NormalizedQuaternion.X;
    YY = NormalizedQuaternion.Y * NormalizedQuaternion.Y;
    ZZ = NormalizedQuaternion.Z * NormalizedQuaternion.Z;
    XY = NormalizedQuaternion.X * NormalizedQuaternion.Y;
    XZ = NormalizedQuaternion.X * NormalizedQuaternion.Z;
    YZ = NormalizedQuaternion.Y * NormalizedQuaternion.Z;
    WX = NormalizedQuaternion.W * NormalizedQuaternion.X;
    WY = NormalizedQuaternion.W * NormalizedQuaternion.Y;
    WZ = NormalizedQuaternion.W * NormalizedQuaternion.Z;

    Result.Elements[0][0] = 1.0f - 2.0f * (YY + ZZ);
    Result.Elements[0][1] = 2.0f * (XY + WZ);
    Result.Elements[0][2] = 2.0f * (XZ - WY);
    Result.Elements[0][3] = 0.0f;

    Result.Elements[1][0] = 2.0f * (XY - WZ);
    Result.Elements[1][1] = 1.0f - 2.0f * (XX + ZZ);
    Result.Elements[1][2] = 2.0f * (YZ + WX);
    Result.Elements[1][3] = 0.0f;

    Result.Elements[2][0] = 2.0f * (XZ + WY);
    Result.Elements[2][1] = 2.0f * (YZ - WX);
    Result.Elements[2][2] = 1.0f - 2.0f * (XX + YY);
    Result.Elements[2][3] = 0.0f;

    Result.Elements[3][0] = 0.0f;
    Result.Elements[3][1] = 0.0f;
    Result.Elements[3][2] = 0.0f;
    Result.Elements[3][3] = 1.0f;

    return (Result);
}

// This method taken from Mike Day at Insomniac Games.
// https://d3cw3dd2w32x2b.cloudfront.net/wp-content/uploads/2015/01/matrix-to-quat.pdf
//
// Note that as mentioned at the top of the paper, the paper assumes the matrix
// would be *post*-multiplied to a vector to rotate it, meaning the matrix is
// the transpose of what we're dealing with. But, because our matrices are
// stored in column-major order, the indices *appear* to match the paper.
//
// For example, m12 in the paper is row 1, column 2. We need to transpose it to
// row 2, column 1. But, because the column comes first when referencing
// elements, it looks like M.Elements[1][2].
//
// Don't be confused! Or if you must be confused, at least trust this
// comment. :)
COVERAGE(HMM_Mat4ToQuaternion, 4)
hmm_quaternion HMM_PREFIX(Mat4ToQuaternion)(hmm_mat4 M)
{
    float T;
    hmm_quaternion Q;

    if (M.Elements[2][2] < 0.0f) {
        if (M.Elements[0][0] > M.Elements[1][1]) {
            ASSERT_COVERED(HMM_Mat4ToQuaternion);

            T = 1 + M.Elements[0][0] - M.Elements[1][1] - M.Elements[2][2];
            Q = HMM_PREFIX(Quaternion)(
                T,
                M.Elements[0][1] + M.Elements[1][0],
                M.Elements[2][0] + M.Elements[0][2],
                M.Elements[1][2] - M.Elements[2][1]
            );
        } else {
            ASSERT_COVERED(HMM_Mat4ToQuaternion);

            T = 1 - M.Elements[0][0] + M.Elements[1][1] - M.Elements[2][2];
            Q = HMM_PREFIX(Quaternion)(
                M.Elements[0][1] + M.Elements[1][0],
                T,
                M.Elements[1][2] + M.Elements[2][1],
                M.Elements[2][0] - M.Elements[0][2]
            );
        }
    } else {
        if (M.Elements[0][0] < -M.Elements[1][1]) {
            ASSERT_COVERED(HMM_Mat4ToQuaternion);

            T = 1 - M.Elements[0][0] - M.Elements[1][1] + M.Elements[2][2];
            Q = HMM_PREFIX(Quaternion)(
                M.Elements[2][0] + M.Elements[0][2],
                M.Elements[1][2] + M.Elements[2][1],
                T,
                M.Elements[0][1] - M.Elements[1][0]
            );
        } else {
            ASSERT_COVERED(HMM_Mat4ToQuaternion);

            T = 1 + M.Elements[0][0] + M.Elements[1][1] + M.Elements[2][2];
            Q = HMM_PREFIX(Quaternion)(
                M.Elements[1][2] - M.Elements[2][1],
                M.Elements[2][0] - M.Elements[0][2],
                M.Elements[0][1] - M.Elements[1][0],
                T
            );
        }
    }

    Q = HMM_PREFIX(MultiplyQuaternionF)(Q, 0.5f / HMM_PREFIX(SquareRootF)(T));

    return Q;
}

COVERAGE(HMM_QuaternionFromAxisAngle, 1)
hmm_quaternion HMM_PREFIX(QuaternionFromAxisAngle)(hmm_vec3 Axis, float AngleOfRotation)
{
    ASSERT_COVERED(HMM_QuaternionFromAxisAngle);

    hmm_quaternion Result;

    hmm_vec3 AxisNormalized = HMM_PREFIX(NormalizeVec3)(Axis);
    float SineOfRotation = HMM_PREFIX(SinF)(AngleOfRotation / 2.0f);

    Result.XYZ = HMM_PREFIX(MultiplyVec3f)(AxisNormalized, SineOfRotation);
    Result.W = HMM_PREFIX(CosF)(AngleOfRotation / 2.0f);

    return (Result);
}
