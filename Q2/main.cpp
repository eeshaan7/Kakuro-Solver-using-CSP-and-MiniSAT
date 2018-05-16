/* 
    Kakuro Solver Using MiniSAT
    Eeshaan Sharma
    2015CSB1011
*/

/*
    Useful Header Files
*/
#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
#include <stack>
#include <algorithm>
#include <queue>
#include <ctime>
#include <bits/stdc++.h>

using namespace std;
using std::string;

/*
    Declaring Useful Structs
*/

/*
    Each puzzle is modelled as a
    2-D Matrix of Node type
*/
struct Node {
    int type; // 1 - Blank 2 - Value 3 - Constraint
    int value;
    int column_constraint;
    int row_constraint;
};

Node **Kakuro; // Global 2-D Matrix to store Kakuro Puzzle
int row, col;  // To store rows and columns in matrix

/*
    Construct to model variables
*/
struct Variable {
    pair <int,int>  idx; // Index to which the variable maps
    vector <int> domain; // Domain of variable
};

vector <Variable> Variables; // Global vector to store all variables

/*
    Each constraint is between a pair
    of nodes.
*/
struct Constraint {
    vector < pair<int,int> > idx; // To store indexes to which constraint matches
    int sum; // if type 2 then this stores value of sum
};

vector <Constraint> Constraints; // Global Vector to store all constraints

/*
    Function Prototypes
*/
void Formulate_Puzzle(char *); // To formulate the puzzle by filling Kakuro matrix
void Formulate_Variables(); // To formulate variables in the puzzle 
void Formulate_Constraints(); // To formulate the constraints in the puzzle
void Get_Possible_Assignment(vector < vector<int> > &A, int, int); // To get possible assignments of a variable
void Generate_Possible_Assignments(vector < vector<int> > &A, int, int, vector <int> &temp, int [], int [], int [], int); // Utility Function
int Find_Variable_Idx(pair <int,int> X);

/*
    Main Function
*/
int main(int argc, char *argv[])
{
    // Starting the clock
    clock_t start_time = clock();
    double cpu_time_used;

    // Checking command line argument format
    if (argc != 3)
    {
        cout<<"The correct syntax is - mysolution <input file> <output file>\n";
    }

    else
    {
        srand(time(NULL));
        
        system ("cd MiniSat_v1.14 && make");

        char *input_file;
        input_file = argv[1]; // Input file name
        
        Formulate_Puzzle(input_file); // Extracting Puzzle Informationtion
        //cout<<"Puzzle formed"<<endl;

        Formulate_Variables(); // Formulating the variables
        //cout<<"Variables formed"<<endl;
        
        Formulate_Constraints(); // Formulating the constraints
        //cout<<"Constraints formed"<<endl;

        // Constructing File with CNF Clauses
        ofstream fout;
        fout.open("CNF_Clauses.txt");

        int no_CNF_clauses = 0; // To keep count of the number of CNF clauses formed

        // Variable constraint taking or of all values (atleast one from 1-9 is there in a cell)
        for(int i=0;i<Variables.size();i++)
        {
            for(int j=0;j<Variables[i].domain.size();j++)
            {
                fout<<i+1<<Variables[i].domain[j]<<" ";
            }
            fout<<"0\n";
            no_CNF_clauses++;
        }

        // Variable constraint such that no variable takes more than 1 value
        for(int i=0;i<Variables.size();i++)
        {
            for(int j=0;j<Variables[i].domain.size();j++)
            {
                for(int k=j+1;k<Variables[i].domain.size();k++)
                {
                    fout<<"-"<<i+1<<Variables[i].domain[j]<<" "<<"-"<<i+1<<Variables[i].domain[k]<<" 0\n";
                    no_CNF_clauses++;
                }   
            }
        }

        int index = Variables.size() + 1;

        int v = 0;

        // Formulating CNF Clauses for all sum constraints
        for(int i=0;i<Constraints.size();i++)
        {
            vector <vector <int> > Assignment;
            Get_Possible_Assignment(Assignment, Constraints[i].idx.size(), Constraints[i].sum);
            
            v += Assignment.size();

            int pos = index + i;
            for(int j=0;j<Assignment.size();j++)
            {
                fout<<pos<<j<<" ";
            }
            fout<<"0\n";
            no_CNF_clauses++;

            for(int j=0;j<Assignment.size();j++)
            {
                for(int k=0;k<Assignment[j].size();k++)
                {
                    fout<<"-"<<pos<<j<<" ";
                    int var_pos = Find_Variable_Idx(Constraints[i].idx[k]);
                    fout<<var_pos+1<<Assignment[j][k]<<" 0\n";
                    no_CNF_clauses++;
                    /*
                    fout<<pos<<j<<" ";
                    fout<<"-"<<Constraints[i].idx[k].first<<Constraints[i].idx[k].second<<Assignment[j][k]<<" 0\n";
                    no_CNF_clauses++;
                    */
                }
            }
        }

        //cout<<"Number of CNF Clauses = "<<no_CNF_clauses<<endl;

        fout.close();

        ofstream fout2;
        fout2.open("CNF.txt");

        ifstream fin2;
        fin2.open("CNF_Clauses.txt");

        int no_Variables = (Variables.size() * 9) + v;
        

        fout2<<"p cnf "<<no_Variables<<" "<<no_CNF_clauses<<endl;

        string st;

        while (fin2)
        {
            getline(fin2, st);
            fout2<<st<<endl;
        }

        fout2.close();
        fin2.close();

        // Call minisat from cnf.txt
        system ("./MiniSat_v1.14/minisat CNF.txt satsolution.txt");

        // Parse output from minisat
        ifstream fin3;
        fin3.open("satsolution.txt");

        fin3 >> st;
        int x;

        ofstream fout3;
        char *op_file = argv[2];
        fout3.open(op_file);

        if (st == "UNSAT")
        {
            fout3<<"Unsolvable Puzzle\n";
        }
        else 
        {
            vector <int> V;
            while (fin3 >> x)
            {
                if (abs(x) < 11)
                {
                    continue;
                }
                if (x > 0)
                {
                    int val = x%10;
                    x /= 10;
                    V.push_back(val);
                    //cout<<x<<" "<<val<<endl;
                }
            }
            // Writing Output to file
        
            fout3<<"rows="<<row<<endl;
            fout3<<"columns="<<col<<endl;
            fout3<<"Horizontal"<<endl;

            // Printing Horizontal Solution for Puzzle
            int var_idx = 0;
            for(int i=0;i<row;i++)
            {
                for(int j=0;j<col-1;j++)
                {
                    if(Kakuro[i][j].type == 1)
                    {
                        fout3<<"#,";
                    }
                    else if(Kakuro[i][j].type == 2)
                    {
                        int value = 0;
                        value = V[var_idx];
                        var_idx++;
                        fout3<<value<<",";
                    }
                    else 
                    {
                        if(Kakuro[i][j].row_constraint != -1)
                            fout3<<Kakuro[i][j].row_constraint<<",";
                        else
                            fout3<<"#,";
                    }
                }

                if(Kakuro[i][col-1].type == 1)
                {
                    fout3<<"#\n";
                }
                else if(Kakuro[i][col-1].type == 2)
                {
                    int value = 0;
                    value = V[var_idx];
                    var_idx++;
                    fout3<<value<<"\n";
                }
                else 
                {
                    if(Kakuro[i][col-1].row_constraint != -1)
                        fout3<<Kakuro[i][col-1].row_constraint<<"\n";
                    else
                        fout3<<"#\n";
                }
            }

            fout3<<"Vertical"<<endl;
            var_idx = 0;

            // Printing Vertical Solution for Puzzle
            for(int i=0;i<row;i++)
            {
                for(int j=0;j<col-1;j++)
                {
                    if(Kakuro[i][j].type == 1)
                    {
                        fout3<<"#,";
                    }
                    else if(Kakuro[i][j].type == 2)
                    {
                        int value = 0;
                        value = V[var_idx];
                        var_idx++;
                        fout3<<value<<",";
                    }
                    else 
                    {
                        if(Kakuro[i][j].column_constraint != -1)
                            fout3<<Kakuro[i][j].column_constraint<<",";
                        else
                            fout3<<"#,";
                    }
                }

                if(Kakuro[i][col-1].type == 1)
                {
                    fout3<<"#\n";
                }
                else if(Kakuro[i][col-1].type == 2)
                {
                    int value = 0;
                    value = V[var_idx];
                    var_idx++;
                    fout3<<value<<"\n";
                }
                else 
                {
                    if(Kakuro[i][col-1].column_constraint != -1)
                        fout3<<Kakuro[i][col-1].column_constraint<<"\n";
                    else
                        fout3<<"#\n";
                }
            }
        }
        remove("CNF_Clauses.txt");
        
    }

    return 0;
}

/*
    Function Definitions
*/
void Formulate_Puzzle(char *file_name) // To formulate the puzzle by filling Kakuro matrix
{
    // Opening Input File
    ifstream fin;
    fin.open(file_name);

    if (fin == 0)
    {
        cout<<"Error in Opening File\n";
        return;
    }

    // Reading input file to extract useful information
    string str;
    
    // Extracting number of rows 
    row = 0;
    fin >> str;
    for(int i=0;i<str.length();i++)
    {
        if(str[i] >= '0' && str[i] <= '9')
        {
            int dig = str[i] - '0';
            row = (row*10) + dig;
        }
    }

    // Extracting number of columns 
    col = 0;
    fin >> str;
    for(int i=0;i<str.length();i++)
    {
        if(str[i] >= '0' && str[i] <= '9')
        {
            int dig = str[i] - '0';
            col = (col*10) + dig;
        }
    }

    // Initializing Kakuro Puzzle
    Kakuro = new Node*[row];
    for(int i=0;i<row;i++)
    {
        Kakuro[i] = new Node[col];    
    }
    
    
    // Discarding line with word Horizontal
    fin >> str;

    // Extracting Horizontal Information
    int r = 0;
    while(r < row)
    {       
        fin >> str;
        int c = 0;
        // Filling row in puzzle
        int i = 0;
        while(i < str.length())
        {
            string temp = "";
            while(str[i] != ',' && i < str.length())
            {
                temp += str[i];
                i++;    
            }
            
            if(temp == "#") // Blank Node
            {
                Kakuro[r][c].type = 1;
                Kakuro[r][c].value = -1;
                Kakuro[r][c].column_constraint = -1;
                Kakuro[r][c].row_constraint = -1; 
            }
            
            else if(temp == "0") // Value Node
            {
                Kakuro[r][c].type = 2;
                Kakuro[r][c].value = 0;
                Kakuro[r][c].column_constraint = -1;
                Kakuro[r][c].row_constraint = -1;        
            }

            else // Row Constraint Node
            {
                Kakuro[r][c].type = 3;
                Kakuro[r][c].value = -1;
                Kakuro[r][c].column_constraint = -1;
                int val = 0;
                for(int i=0;i<temp.length();i++)
                {
                    int dig = temp[i] - '0';
                    val = (val*10) + dig; 
                }
                Kakuro[r][c].row_constraint = val;   
            }

            i++; // Ignoring Comma Character
            c++;
        }
        
        r++;
    }

    // Discarding line with word Vertical
    fin >> str;

    // Extracting Vertical Information
    r = 0;
    while(r < row)
    {       
        fin >> str;
        int c = 0;
        // Filling row in puzzle
        int i = 0;
        while(i < str.length())
        {
            string temp = "";
            while(str[i] != ',' && i < str.length())
            {
                temp += str[i];
                i++;    
            }
            
            if(temp == "#" && Kakuro[r][c].row_constraint == -1) // Blank Node
            {
                Kakuro[r][c].type = 1;
                Kakuro[r][c].value = -1;
                Kakuro[r][c].column_constraint = -1;
                Kakuro[r][c].row_constraint = -1; 
            }
            
            else if(temp == "0") // Value Node
            {
                Kakuro[r][c].type = 2;
                Kakuro[r][c].value = 0;
                Kakuro[r][c].column_constraint = -1;
                Kakuro[r][c].row_constraint = -1;        
            }

            else if(temp != "#") // Column Constraint Node
            {
                Kakuro[r][c].type = 3;
                Kakuro[r][c].value = -1;
                int val = 0;
                for(int i=0;i<temp.length();i++)
                {
                    int dig = temp[i] - '0';
                    val = (val*10) + dig; 
                }
                Kakuro[r][c].column_constraint = val;   
            }

            i++; // Ignoring Comma Character
            c++;
        }
        
        r++;
    }
}

void Formulate_Variables() // To formulate variables in the puzzle
{
    for(int i=0;i<row;i++)
    {
        for(int j=0;j<col;j++)
        {
            if(Kakuro[i][j].type == 2) // Value Node
            {
                Variable temp;
                temp.idx = make_pair(i,j);
                
                for(int x=1;x<=9;x++)
                {
                    temp.domain.push_back(x);
                }

                Variables.push_back(temp);
            }        
        }
    }
}

void Formulate_Constraints() // To formulate the constraints in the puzzle
{
    // Finding all row and column constraints
    for(int i=0;i<row;i++)
    {
        for(int j=0;j<col;j++)
        {
            if(Kakuro[i][j].type == 3) // Constraint Node
            {
                if(Kakuro[i][j].row_constraint != -1) // Row Constraint
                {
                    pair <int, int> St_Idx, End_Idx; 
                    St_Idx = make_pair(i,j+1);
                    
                    int n = 1; // Degree of constraint
                    int k = j+2;
                    while(k < col && Kakuro[i][k].type == 2) // Till its a value node
                    {
                        k++;
                        n++;
                    }

                    End_Idx = make_pair(i,k-1);

                    Constraint temp;
                    temp.sum = Kakuro[i][j].row_constraint;

                    for(int x=0;x<n;x++)
                    {
                        temp.idx.push_back(make_pair(i,j+x+1));
                    }

                    Constraints.push_back(temp);
                }                

                if(Kakuro[i][j].column_constraint != -1) // Column Constraint
                {
                    pair <int, int> St_Idx, End_Idx; 
                    St_Idx = make_pair(i+1,j);
                    
                    int n = 1; // Degree of constraint
                    int k = i+2;
                    while(k < row && Kakuro[k][j].type == 2) // Till its a value node
                    {
                        k++;
                        n++;
                    }

                    End_Idx = make_pair(k-1,j);

                    Constraint temp;
                    temp.sum = Kakuro[i][j].column_constraint;

                    for(int x=0;x<n;x++)
                    {
                        temp.idx.push_back(make_pair(i+x+1,j));
                    }

                    Constraints.push_back(temp);
                }     
            }       
        }
    }
}

void Get_Possible_Assignment(vector < vector<int> > &A, int size, int sum)
{
    if(size <= 4)
    {
        vector <int> temp1;
        int B[size];
        int C[size];
        int D[size];

        for(int i=0;i<size;i++)
        {
            B[i] = 1;
            C[i] = 0;
            D[i] = 9;
        }

        Generate_Possible_Assignments(A, size, 0, temp1, C, B, D, 0);
    }

    else
    {
        int ctr = 0;
        while(ctr < size)
        {
            vector < vector<int> > temp;
            vector <int> temp1;
            int B[4];
            int C[4];
            int D[4];

            for(int i=0;i<4;i++)
            {
                B[i] = 1;
                C[i] = 0;
                D[i] = 9;
            }

            if(size - ctr >= 4)
            {
                Generate_Possible_Assignments(temp, 4, 0, temp1, C, B, D, ctr);
            }              
            else
            {
                Generate_Possible_Assignments(temp, size - ctr, 0, temp1, C, B, D, ctr);
            }

            if(A.size() == 0)
            {
                for(int i=0;i<temp.size();i++)
                {
                    A.push_back(temp[i]);
                }
            }

            else
            {
                vector < vector<int> > R;
                for(int i=0;i<A.size();i++)
                {
                    vector <int> temp2;
                    for(int j=0;j<A[i].size();j++)
                    {
                        temp2.push_back(A[i][j]);
                    }
                    for(int k=0;k<temp.size();k++)
                    {
                        for(int l=0;l<temp[k].size();l++)
                        {
                            temp2.push_back(temp[k][l]);
                        }
                        
                        R.push_back(temp2);

                        for(int l=0;l<temp[k].size();l++)
                        {
                            temp2.pop_back();
                        }
                    } 
                }

                int size = A.size();
                for(int i=0;i<size;i++)
                {
                    A.pop_back();
                }

                for(int i=0;i<R.size();i++)
                {
                    A.push_back(R[i]);
                }    
            }

            ctr += 4;        
        }
    }

    // Removing assignments which do not sum up to given sum
    vector < vector<int> > X;

    for(int i=0;i<A.size();i++)
    {
        int S = 0;
        for(int j=0;j<A[i].size();j++)
        {
            S += A[i][j];
        }

        if(S == sum)
        {
            // Checking for duplicates
            int flag = 1;
            for(int j=0;j<A[i].size();j++)
            {
                for(int k=j+1;k<A[i].size();k++)
                {
                    if(A[i][j] == A[i][k])
                    {
                        flag--;
                        break;
                    }
                }
                if(flag == 0)
                {
                    break;
                }
            }

            if(flag == 1)
            {
                X.push_back(A[i]);
            }
        }
    }

    int siz = A.size();
    for(int i=0;i<siz;i++)
    {
        A.pop_back();
    }

    for(int i=0;i<X.size();i++)
    {
        A.push_back(X[i]);
    }
}

void Generate_Possible_Assignments(vector < vector<int> > &A, int max_idx, int curr_idx, vector <int> &temp, int C[], int B[], int D[], int from_idx)
{   
    if(curr_idx < 0 || curr_idx > max_idx)
    {
        return;
    }

    if(C[0] == 1)
    {
        return;
    }

    if(curr_idx < max_idx && B[curr_idx] > D[curr_idx])
    {
        C[curr_idx] += 1;
        B[curr_idx] = 1;
        if(curr_idx > 0)
            B[curr_idx-1] += 1;
        temp.pop_back();
        Generate_Possible_Assignments(A, max_idx, curr_idx-1, temp, C, B, D, from_idx);
    }

    else if(max_idx == curr_idx)
    {
        A.push_back(temp);

        temp.pop_back();
        B[curr_idx-1] += 1;
        Generate_Possible_Assignments(A, max_idx, curr_idx-1, temp, C ,B, D, from_idx);
    }

    else
    {
        temp.push_back(B[curr_idx]);
        Generate_Possible_Assignments(A, max_idx, curr_idx+1, temp, C, B, D, from_idx);
    }
}  

int Find_Variable_Idx(pair <int,int> X)
{
    int pos = -1;
    for(int i=0;i<Variables.size();i++)
    {
        if(Variables[i].idx.first == X.first && Variables[i].idx.second == X.second)
        {
            pos = i;
            break;
        }
    }
    return pos;
}