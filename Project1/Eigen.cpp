# include <cstdlib>
# include <iostream>
# include <iomanip>
# include <cmath>
# include <ctime>
# include <cstring>
#include <fstream>
#include <string>
#include <chrono>

using namespace std;

# include "jacobi_eigenvalue.hpp"

//****************************************************************************80

void jacobi_eigenvalue(int n, double a[], int it_max, double v[], double d[], int& it_num, int& rot_num)

//****************************************************************************80
//  Parameters:
//
//    Input, int N, the order of the matrix.
//
//    Input, double A[N*N], the matrix, which must be square, real,
//    and symmetric.
//
//    Input, int IT_MAX, the maximum number of iterations.
//
//    Output, double V[N*N], the matrix of eigenvectors.
//
//    Output, double D[N], the eigenvalues, in descending order.
//
//    Output, int &IT_NUM, the total number of iterations.
//
//    Output, int &ROT_NUM, the total number of rotations.
//
{
    double* bw;
    double c;
    double g;
    double gapq;
    double h;
    int i;
    int j;
    int k;
    int l;
    int m;
    int p;
    int q;
    double s;
    double t;
    double tau;
    double term;
    double termp;
    double termq;
    double theta;
    double sum_for_stop_rotate;
    double w;
    double* zw;

    r8mat_identity(n, v);  // sets square of v to be 1, identity matrix

    r8mat_diag_get_vector(n, a, d);  // diagonal entries of the matrix a returns in d

    bw = new double[n];
    zw = new double[n];

    for (i = 0; i < n; i++)
    {
        bw[i] = d[i];
        zw[i] = 0.0;
    }
    it_num = 0;
    rot_num = 0;

    while (it_num < it_max)
    {
        it_num = it_num + 1;
        //
        //  The convergence sum_for_stop_rotateold is based on the size of the elements in
        //  the strict upper triangle of the matrix.
        //
        sum_for_stop_rotate = 0.0;
        for (j = 0; j < n; j++)
        {
            for (i = 0; i < j; i++)
            {
                sum_for_stop_rotate = sum_for_stop_rotate + a[i + j * n] * a[i + j * n];
            }
        }

        sum_for_stop_rotate = sqrt(sum_for_stop_rotate) / (double)(4 * n);

        if (sum_for_stop_rotate == 0.0)
        {
            break;
        }

        for (p = 0; p < n; p++)
        {
            for (q = p + 1; q < n; q++)
            {
                gapq = 10.0 * fabs(a[p + q * n]);
                termp = gapq + fabs(d[p]);
                termq = gapq + fabs(d[q]);
                //
                //  Annihilate tiny offdiagonal elements.
                //
                if (4 < it_num && termp == fabs(d[p]) && termq == fabs(d[q]))
                {
                    a[p + q * n] = 0.0;
                }
                //
                //  Otherwise, apply a rotation.
                //
                else if (sum_for_stop_rotate <= fabs(a[p + q * n]))
                {
                    h = d[q] - d[p];
                    term = fabs(h) + gapq;

                    if (term == fabs(h))
                    {
                        t = a[p + q * n] / h;
                    }
                    else  // theta calc
                    {
                        theta = 0.5 * h / a[p + q * n];
                        t = 1.0 / (fabs(theta) + sqrt(1.0 + theta * theta));
                        if (theta < 0.0)
                        {
                            t = -t;
                        }
                    }
                    c = 1.0 / sqrt(1.0 + t * t);
                    s = t * c;
                    tau = s / (1.0 + c);
                    h = t * a[p + q * n];
                    //
                    //  Accumulate corrections to diagonal elements.
                    //
                    zw[p] = zw[p] - h;
                    zw[q] = zw[q] + h;
                    d[p] = d[p] - h;
                    d[q] = d[q] + h;

                    a[p + q * n] = 0.0;
                    //
                    //  Rotate, using information from the upper triangle of A only.
                    //
                    for (j = 0; j < p; j++)
                    {
                        g = a[j + p * n];
                        h = a[j + q * n];
                        a[j + p * n] = g - s * (h + g * tau);
                        a[j + q * n] = h + s * (g - h * tau);
                    }

                    for (j = p + 1; j < q; j++)
                    {
                        g = a[p + j * n];
                        h = a[j + q * n];
                        a[p + j * n] = g - s * (h + g * tau);
                        a[j + q * n] = h + s * (g - h * tau);
                    }

                    for (j = q + 1; j < n; j++)
                    {
                        g = a[p + j * n];
                        h = a[q + j * n];
                        a[p + j * n] = g - s * (h + g * tau);
                        a[q + j * n] = h + s * (g - h * tau);
                    }
                    //
                    //  Accumulate information in the eigenvector matrix.
                    //
                    for (j = 0; j < n; j++)
                    {
                        g = v[j + p * n];
                        h = v[j + q * n];
                        v[j + p * n] = g - s * (h + g * tau);
                        v[j + q * n] = h + s * (g - h * tau);
                    }
                    rot_num = rot_num + 1;
                }
            }
        }

        for (i = 0; i < n; i++)
        {
            bw[i] = bw[i] + zw[i];
            d[i] = bw[i];
            zw[i] = 0.0;
        }
    }
    //
    //  Restore upper triangle of input matrix.
    //
    for (j = 0; j < n; j++)
    {
        for (i = 0; i < j; i++)
        {
            a[i + j * n] = a[j + i * n];
        }
    }
    //
    //  Ascending sort the eigenvalues and eigenvectors.
    //
    for (k = 0; k < n - 1; k++)
    {
        m = k;
        for (l = k + 1; l < n; l++)
        {
            if (d[l] < d[m])
            {
                m = l;
            }
        }

        if (m != k)
        {
            t = d[m];
            d[m] = d[k];
            d[k] = t;
            for (i = 0; i < n; i++)
            {
                w = v[i + m * n];
                v[i + m * n] = v[i + k * n];
                v[i + k * n] = w;
            }
        }
    }

    delete[] bw;
    delete[] zw;

    return;
}
//****************************************************************************80

void r8mat_diag_get_vector(int n, double a[], double v[])

//****************************************************************************80
//
//  Purpose:
//
//    R8MAT_DIAG_GET_VECTOR gets the value of the diagonal of an R8MAT.
//
//  Discussion:
//
//    An R8MAT is a doubly dimensioned array of R8 values, stored as a vector
//    in column-major order.
//

//  Parameters:
//
//    Input, int N, the number of rows and columns of the matrix.
//
//    Input, double A[N*N], the N by N matrix.
//
//    Output, double V[N], the diagonal entries
//    of the matrix.
//
{
    int i;

    for (i = 0; i < n; i++)
    {
        v[i] = a[i + i * n];
    }

    return;
}
//****************************************************************************80

void r8mat_identity(int n, double a[])

//****************************************************************************80
//
//  Purpose:
//
//    R8MAT_IDENTITY sets the square matrix A to the identity.
//
//  Discussion:
//
//    An R8MAT is a doubly dimensioned array of R8 values, stored as a vector
//    in column-major order.
//
//  Parameters:
//
//    Input, int N, the order of A.
//
//    Output, double A[N*N], the N by N identity matrix.
//
{
    int i;
    int j;
    int k;

    k = 0;
    for (j = 0; j < n; j++)
    {
        for (i = 0; i < n; i++)
        {
            if (i == j)
            {
                a[k] = 1.0;
            }
            else
            {
                a[k] = 0.0;
            }
            k = k + 1;
        }
    }

    return;
}
//****************************************************************************80




void r8mat_print(int m, int n, double a[], string title)

//****************************************************************************80
//
//  Purpose:
//
//    R8MAT_PRINT prints an R8MAT.
//
//  Discussion:
//
//    An R8MAT is a doubly dimensioned array of R8 values, stored as a vector
//    in column-major order.
//
//    Entry A(I,J) is stored as A[I+J*M]
//
//
//  Parameters:
//
//    Input, int M, the number of rows in A.
//
//    Input, int N, the number of columns in A.
//
//    Input, double A[M*N], the M by N matrix.
//
//    Input, string TITLE, a title.
//
{
    r8mat_print_some(m, n, a, 1, 1, m, n, title);

    return;
}
//****************************************************************************80

void r8mat_print_some(int m, int n, double a[], int ilo, int jlo, int ihi,
    int jhi, string title)

    //****************************************************************************80
    //
    //  Purpose:
    //
    //    R8MAT_PRINT_SOME prints some of an R8MAT.
    //
    //  Discussion:
    //
    //    An R8MAT is a doubly dimensioned array of R8 values, stored as a vector
    //    in column-major order.
    //
    //  Parameters:
    //
    //    Input, int M, the number of rows of the matrix.
    //    M must be positive.
    //
    //    Input, int N, the number of columns of the matrix.
    //    N must be positive.
    //
    //    Input, double A[M*N], the matrix.
    //
    //    Input, int ILO, JLO, IHI, JHI, designate the first row and
    //    column, and the last row and column to be printed.
    //
    //    Input, string TITLE, a title.
    //
{
# define INCX 5

    int i;
    int i2hi;
    int i2lo;
    int j;
    int j2hi;
    int j2lo;

    cout << "\n";
    cout << title << "\n";

    if (m <= 0 || n <= 0)
    {
        cout << "\n";
        cout << "  (None)\n";
        return;
    }
    //
    //  Print the columns of the matrix, in strips of 5.
    //
    for (j2lo = jlo; j2lo <= jhi; j2lo = j2lo + INCX)
    {
        j2hi = j2lo + INCX - 1;
        if (n < j2hi)
        {
            j2hi = n;
        }
        if (jhi < j2hi)
        {
            j2hi = jhi;
        }
        cout << "\n";
        //
        //  For each column J in the current range...
        //
        //  Write the header.
        //
        cout << "  Col:    ";
        for (j = j2lo; j <= j2hi; j++)
        {
            cout << setw(7) << j - 1 << "       ";
        }
        cout << "\n";
        cout << "  Row\n";
        cout << "\n";
        //
        //  Determine the range of the rows in this strip.
        //
        if (1 < ilo)
        {
            i2lo = ilo;
        }
        else
        {
            i2lo = 1;
        }
        if (ihi < m)
        {
            i2hi = ihi;
        }
        else
        {
            i2hi = m;
        }

        for (i = i2lo; i <= i2hi; i++)
        {
            //
            //  Print out (up to) 5 entries in row I, that lie in the current strip.
            //
            cout << setw(5) << i - 1 << ": ";
            for (j = j2lo; j <= j2hi; j++)
            {
                cout << setw(12) << a[i - 1 + (j - 1) * m] << "  ";
            }
            cout << "\n";
        }
    }

    return;
# undef INCX
}
//****************************************************************************80

void r8vec_print(int n, double a[], string title)

//****************************************************************************80
//
//  Purpose:
//
//    R8VEC_PRINT prints an R8VEC.
//
//  Discussion:
//
//    An R8VEC is a vector of R8's.
//  Parameters:
//
//    Input, int N, the number of components of the vector.
//
//    Input, double A[N], the vector to be printed.
//
//    Input, string TITLE, a title.
//
{
    int i;

    cout << "\n";
    cout << title << "\n";
    cout << "\n";
    for (i = 0; i < n; i++)
    {
        cout << "  " << setw(8) << i
            << ": " << setw(14) << a[i] << "\n";
    }

    return;
}


using namespace std;

const int SIZE = 25;
double grades[SIZE];
double eigenvec[SIZE];
double eigenval[SIZE];
void readData() {

    string inFileName = "data_2.txt";
    ifstream inFile;
    inFile.open(inFileName.c_str());

    if (inFile.is_open())
    {
        for (int i = 0; i < SIZE; i++)
        {
            inFile >> grades[i];
            /*cout << grades[i] << " ";*/

        }
        cout << " \n\n";
        inFile.close(); // CLose input file
    }
    else { //Error message
        cerr << "Can't find input file " << inFileName << endl;
    }
}

//void jacobi_eigenvalue(int n, double a[], int it_max, double v[],double d[], int& it_num, int& rot_num)

int main() {
    auto start = chrono::steady_clock::now();
    int a = 5, b = 7, count = 0, n = 5, index = 1;
    readData();
    int* it_num = &a;
    int* rot_num = &b;
    jacobi_eigenvalue(n, grades, 25, eigenvec, eigenval, *it_num, *rot_num);
    auto end = chrono::steady_clock::now();
    cout << "algorithm finished.";

    r8mat_print(n, n, eigenvec, "eigen vectors:");
    r8vec_print(n, eigenval, "eigen values:");
    cout << "Time reading matrix: "
        << chrono::duration_cast<chrono::nanoseconds>(end - start).count() / pow(10, 9)
        << " [SEC]" << endl;
    return 0;
}