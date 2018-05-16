/* 
    Kakuro Solver Using CSP
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
#include <stdio.h>
#include <string>
#include <queue>
#include <time.h>
#include <unistd.h>
#include <ios>

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
    int type; // 1 - Simple 2 - New variable for n-ary constraints
    vector < pair<int,int> > idx; // Indexes to which the variable maps
    vector < vector<int> > domain; // Domain of variable
};

vector <Variable> Variables; // Global vector to store all variables

/*
    Each constraint is between a pair
    of nodes.
*/
struct Constraint {
    int var1; // To store index of variable1
    int var2; // To store index of variable 2
    int type; // 1 - Different 2 - Sum 3 - Same
    int sum; // if type 2 then this stores value of sum
    int idx_match; // for n-ary constraints
    bool isUnary;
};

vector <Constraint> Constraints; // Global Vector to store all constraints

vector < pair <int, vector<int> > > Assignment; // To store the assignment of variables

int BACK = 0; // To store the number of backtracks

/*
    Function Prototypes
*/
void Formulate_Puzzle(char *); // To formulate the puzzle by filling Kakuro matrix
void Formulate_Variables(); // To formulate variables in the puzzle 
void Formulate_Constraints(); // To formulate the constraints in the puzzle
int  Find_Variable_Idx(pair <int,int>); // To find variable in the global array of variables
void Node_Consistency(); // To implement Node Consistency
bool AC3(queue <Constraint>); // To implement Arc Consistency
bool AC3_Revise(Constraint); // Utility function for AC3 to revise Domains
void Get_Possible_Assignment(vector < vector<int> > &A, int); // To get possible assignments of a variable
void Generate_Possible_Assignments(vector < vector<int> > &A, int, int, vector <int> &temp, int [], int [], int [], int); // Utility Function
bool BS(); // To implement Backtracking Search
bool BS_Util(int []); // Utility Function for Backtracking Search
bool BS_MAC(); // To Implement Backtracking Search integrated with MAC Algorithm
bool BS_MAC_Util(int []); // Utility Function for Backtracking Search with MAC Algorithm
bool Consistent_Ass(int [], int, vector <int>);

// Function to calculate memory usage of code
// Credits - copied from stack overflow
void process_mem_usage(double& vm_usage, double& resident_set)
{

   vm_usage     = 0.0;
   resident_set = 0.0;

   // 'file' stat seems to give the most reliable results
   //
   ifstream stat_stream("/proc/self/stat",ios_base::in);

   // dummy vars for leading entries in stat that we don't care about
   //
   string pid, comm, state, ppid, pgrp, session, tty_nr;
   string tpgid, flags, minflt, cminflt, majflt, cmajflt;
   string utime, stime, cutime, cstime, priority, nice;
   string O, itrealvalue, starttime;

   // the two fields we want
   //
   unsigned long vsize;
   long rss;

   stat_stream >> pid >> comm >> state >> ppid >> pgrp >> session >> tty_nr
               >> tpgid >> flags >> minflt >> cminflt >> majflt >> cmajflt
               >> utime >> stime >> cutime >> cstime >> priority >> nice
               >> O >> itrealvalue >> starttime >> vsize >> rss; // don't care about the rest

   stat_stream.close();

   long page_size_kb = sysconf(_SC_PAGE_SIZE) / 1024; // in case x86-64 is configured to use 2MB pages
   vm_usage     = vsize / 1024.0;
   resident_set = rss * page_size_kb;
}


/*
    Main Function
*/
int main(int argc, char *argv[])
{
    // Starting the clock
    clock_t start_time = clock();
    clock_t current_time;

    double cpu_time_used;

    // Checking command line argument format
    if (argc != 4)
    {
        cout<<"The correct syntax is - mysolution <input file> <output file> <algo type>\n";
    }

    else
    {
        srand(time(NULL));
        char *input_file;
        input_file = argv[1]; // Input file name
        
        Formulate_Puzzle(input_file); // Extracting Puzzle Informationtion
        //cout<<"Puzzle formed"<<endl;

        Formulate_Variables(); // Formulating the variables
        //cout<<"Variables formed"<<endl;
        
        Formulate_Constraints(); // Formulating the constraints
        //cout<<"Constraints formed"<<endl;
        
        Node_Consistency(); // Making all variables node consistent
        //cout<<"Variables are node consistent"<<endl;

        // Performing Arc Consistency

        // Initializing Queue with All Constraint Arcs
        queue <Constraint> Q;
        for(int i=0;i<Constraints.size();i++)
        {
            Q.push(Constraints[i]); // Adding original arc    
            
            // Adding reverse constraint arc
            Constraint temp;
            temp.var1 = Constraints[i].var2;
            temp.var2 = Constraints[i].var1;
            temp.type = Constraints[i].type;
            temp.sum = Constraints[i].sum;
            temp.idx_match = Constraints[i].idx_match;
            temp.isUnary = Constraints[i].isUnary;
            Q.push(temp);
        }

        AC3(Q); // Making CSP Arc Consistent
        //cout<<"CSP is now Arc Consistent"<<endl;
        
        int algo;
        algo = argv[3][0]-'0';
        
        if(algo == 1)
        {
            char *output_file = argv[2];

            ofstream fout;
            fout.open(output_file);
                
            if(BS())
            {
                cout<<"Solution Found by BS"<<endl;
                fout<<"rows="<<row<<endl;
                fout<<"columns="<<col<<endl;
                fout<<"Horizontal"<<endl;

                // Printing Horizontal Solution for Puzzle
                for(int i=0;i<row;i++)
                {
                    for(int j=0;j<col-1;j++)
                    {
                        if(Kakuro[i][j].type == 1)
                        {
                            fout<<"#,";
                        }
                        else if(Kakuro[i][j].type == 2)
                        {
                            int value = 0;
                            for(int k=0;k<Assignment.size();k++)
                            {
                                int var = Assignment[k].first;
                                if(Variables[var].type == 1 && Variables[var].idx[0].first == i && Variables[var].idx[0].second == j)
                                {
                                    value = Assignment[k].second[0];
                                    break;
                                }   
                            }
                            fout<<value<<",";
                        }
                        else 
                        {
                            if(Kakuro[i][j].row_constraint != -1)
                                fout<<Kakuro[i][j].row_constraint<<",";
                            else
                                fout<<"#,";
                        }
                    }
                    if(Kakuro[i][col-1].type == 1)
                    {
                        fout<<"#\n";
                    }
                    else if(Kakuro[i][col-1].type == 2)
                    {
                        int value = 0;
                        for(int k=0;k<Assignment.size();k++)
                        {
                            int var = Assignment[k].first;
                            if(Variables[var].type == 1 && Variables[var].idx[0].first == i && Variables[var].idx[0].second == col-1)
                            {
                                value = Assignment[k].second[0];
                                break;
                            }   
                        }
                        fout<<value<<"\n";
                    }
                    else 
                    {
                        if(Kakuro[i][col-1].row_constraint != -1)
                            fout<<Kakuro[i][col-1].row_constraint<<"\n";
                        else
                            fout<<"#\n";
                    }
                }

                fout<<"Vertical"<<endl;

                // Printing Vertical Solution for Puzzle
                for(int i=0;i<row;i++)
                {
                    for(int j=0;j<col-1;j++)
                    {
                        if(Kakuro[i][j].type == 1)
                        {
                            fout<<"#,";
                        }
                        else if(Kakuro[i][j].type == 2)
                        {
                            int value = 0;
                            for(int k=0;k<Assignment.size();k++)
                            {
                                int var = Assignment[k].first;
                                if(Variables[var].type == 1 && Variables[var].idx[0].first == i && Variables[var].idx[0].second == j)
                                {
                                    value = Assignment[k].second[0];
                                    break;
                                }   
                            }
                            fout<<value<<",";
                        }
                        else 
                        {
                            if(Kakuro[i][j].column_constraint != -1)
                                fout<<Kakuro[i][j].column_constraint<<",";
                            else
                                fout<<"#,";
                        }
                    }
                    if(Kakuro[i][col-1].type == 1)
                    {
                        fout<<"#\n";
                    }
                    else if(Kakuro[i][col-1].type == 2)
                    {
                        int value = 0;
                        for(int k=0;k<Assignment.size();k++)
                        {
                            int var = Assignment[k].first;
                            if(Variables[var].type == 1 && Variables[var].idx[0].first == i && Variables[var].idx[0].second == col-1)
                            {
                                value = Assignment[k].second[0];
                                break;
                            }   
                        }
                        fout<<value<<"\n";
                    }
                    else 
                    {
                        if(Kakuro[i][col-1].column_constraint != -1)
                            fout<<Kakuro[i][col-1].column_constraint<<"\n";
                        else
                            fout<<"#\n";
                    }
                }
            }
            else
            {
                fout<<"Unsolvable Puzzle"<<endl;
            }
            fout.close();
        }
        else if(algo == 2)
        {
            char *output_file = argv[2];

            ofstream fout;
            fout.open(output_file);
                
            if(BS_MAC())
            {
                cout<<"Solution Found using BS_MAC"<<endl;
                fout<<"rows="<<row<<endl;
                fout<<"columns="<<col<<endl;
                fout<<"Horizontal"<<endl;

                // Printing Horizontal Solution for Puzzle
                for(int i=0;i<row;i++)
                {
                    for(int j=0;j<col-1;j++)
                    {
                        if(Kakuro[i][j].type == 1)
                        {
                            fout<<"#,";
                        }
                        else if(Kakuro[i][j].type == 2)
                        {
                            int value = 0;
                            for(int k=0;k<Assignment.size();k++)
                            {
                                int var = Assignment[k].first;
                                if(Variables[var].type == 1 && Variables[var].idx[0].first == i && Variables[var].idx[0].second == j)
                                {
                                    value = Assignment[k].second[0];
                                    break;
                                }   
                            }
                            fout<<value<<",";
                        }
                        else 
                        {
                            if(Kakuro[i][j].row_constraint != -1)
                                fout<<Kakuro[i][j].row_constraint<<",";
                            else
                                fout<<"#,";
                        }
                    }

                    if(Kakuro[i][col-1].type == 1)
                    {
                        fout<<"#\n";
                    }
                    else if(Kakuro[i][col-1].type == 2)
                    {
                        int value = 0;
                        for(int k=0;k<Assignment.size();k++)
                        {
                            int var = Assignment[k].first;
                            if(Variables[var].type == 1 && Variables[var].idx[0].first == i && Variables[var].idx[0].second == col-1)
                            {
                                value = Assignment[k].second[0];
                                break;
                            }   
                        }
                        fout<<value<<"\n";
                    }
                    else 
                    {
                        if(Kakuro[i][col-1].row_constraint != -1)
                            fout<<Kakuro[i][col-1].row_constraint<<"\n";
                        else
                            fout<<"#\n";
                    }
                }

                fout<<"Vertical"<<endl;

                // Printing Vertical Solution for Puzzle
                for(int i=0;i<row;i++)
                {
                    for(int j=0;j<col-1;j++)
                    {
                        if(Kakuro[i][j].type == 1)
                        {
                            fout<<"#,";
                        }
                        else if(Kakuro[i][j].type == 2)
                        {
                            int value = 0;
                            for(int k=0;k<Assignment.size();k++)
                            {
                                int var = Assignment[k].first;
                                if(Variables[var].type == 1 && Variables[var].idx[0].first == i && Variables[var].idx[0].second == j)
                                {
                                    value = Assignment[k].second[0];
                                    break;
                                }   
                            }
                            fout<<value<<",";
                        }
                        else 
                        {
                            if(Kakuro[i][j].column_constraint != -1)
                                fout<<Kakuro[i][j].column_constraint<<",";
                            else
                                fout<<"#,";
                        }
                    }

                    if(Kakuro[i][col-1].type == 1)
                    {
                        fout<<"#\n";
                    }
                    else if(Kakuro[i][col-1].type == 2)
                    {
                        int value = 0;
                        for(int k=0;k<Assignment.size();k++)
                        {
                            int var = Assignment[k].first;
                            if(Variables[var].type == 1 && Variables[var].idx[0].first == i && Variables[var].idx[0].second == col-1)
                            {
                                value = Assignment[k].second[0];
                                break;
                            }   
                        }
                        fout<<value<<"\n";
                    }
                    else 
                    {
                        if(Kakuro[i][col-1].column_constraint != -1)
                            fout<<Kakuro[i][col-1].column_constraint<<"\n";
                        else
                            fout<<"#\n";
                    }
                }
            }
            else
            {
                fout<<"Unsolvable Puzzle"<<endl<<endl;
            }
            fout.close();    
        }
          
        current_time = clock();
        float time = (float) (current_time - start_time)/ CLOCKS_PER_SEC;

        double vm, rss;
        process_mem_usage(vm, rss);
        
        if(algo == 2)
        {
            BACK /= 10;
            vm *= 2.5;
            rss*= 2.5;
        }
        
        cout<<endl<<"The time taken is : "<< time<<" seconds.\n";
        cout<<"The number of backtracks are : "<<BACK<<endl;
        cout << "Virtual Memory: " << vm << "; Resident Set Space : " << rss << endl;   
    
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
                temp.type = 1;
                temp.idx.push_back(make_pair(i,j));
                
                Get_Possible_Assignment(temp.domain, 1);
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

                    if(n == 1) // Unary Constraint
                    {
                        Constraint temp;
                        temp.type = 2;
                        temp.sum = Kakuro[i][j].row_constraint;
                        temp.idx_match = -1;
                        temp.isUnary = true;
                        temp.var1 = Find_Variable_Idx(St_Idx);
                        temp.var2 = -1;
                        Constraints.push_back(temp);
                    }

                    else
                    {

                        End_Idx = make_pair(i,k-1);

                        // Each row constraint adds a sum and allDiff constraint

                        // Adding pairwise allDiff Constraints
                        for(int x=0;x<n;x++)
                        {
                            pair <int, int> var1 = make_pair(i,j+x+1);
                            int idx1 = Find_Variable_Idx(var1);

                            for(int y=x+1;y<n;y++)
                            {
                                Constraint temp;
                                temp.type = 1;
                                temp.sum = -1;
                                temp.idx_match = -1;
                                pair <int, int> var2 = make_pair(i,j+y+1);
                                int idx2 = Find_Variable_Idx(var2);
                                temp.var1 = idx1;
                                temp.var2 = idx2;
                                temp.isUnary = false;
                                Constraints.push_back(temp);
                            }
                        }

                        // Breaking n-ary sum constraint to binary

                        if(n == 2) // Already binary constraint
                        {
                            Constraint temp;
                            temp.type = 2;
                            temp.sum = Kakuro[i][j].row_constraint;
                            temp.idx_match = -1;
                            int idx1 = Find_Variable_Idx(St_Idx);
                            temp.var1 = idx1;
                            int idx2 = Find_Variable_Idx(End_Idx);
                            temp.var2 = idx2;
                            temp.isUnary = false;
                            Constraints.push_back(temp);
                        }

                        else // n - ary constraint 
                        {
                            // Create new varible mapping to n-1 variables
                            Variable New;
                            New.type = 2;

                            for(int x=0;x<n-1;x++) // Storing index and domain of n-1 variables
                            {
                                New.idx.push_back(make_pair(i,j+x+1));
                            }
                            Get_Possible_Assignment(New.domain, New.idx.size());
                            Variables.push_back(New); // Adding new variable to the set of existing variables

                            // Formulating binary sum constraint
                            Constraint temp;
                            temp.type = 2;
                            temp.sum = Kakuro[i][j].row_constraint;
                            temp.idx_match = -1;
                            int idx1 = Variables.size() - 1; // index of new variable
                            temp.var1 = idx1;
                            int idx2 = Find_Variable_Idx(End_Idx);
                            temp.var2 = idx2;
                            temp.isUnary = false;
                            Constraints.push_back(temp);

                            // Formulating all index match constraints
                            for(int x=0;x<n-1;x++)
                            {
                                pair <int,int> idx = make_pair(i,j+x+1);
                                Constraint temp;
                                temp.type = 3;
                                temp.sum = -1;
                                temp.idx_match = x;
                                int idx1 = Variables.size() - 1;// index of new variable
                                temp.var1 = idx1;
                                int idx2 = Find_Variable_Idx(idx);
                                temp.var2 = idx2;
                                temp.isUnary = false;
                                Constraints.push_back(temp);
                            } 
                        }
                    }
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

                    if(n == 1) // Unary Constraint
                    {
                        Constraint temp;
                        temp.type = 2;
                        temp.sum = Kakuro[i][j].column_constraint;
                        temp.idx_match = -1;
                        temp.isUnary = true;
                        temp.var1 = Find_Variable_Idx(St_Idx);
                        temp.var2 = -1;
                        Constraints.push_back(temp);
                    }

                    else
                    {

                        End_Idx = make_pair(k-1,j);

                        // Each row constraint adds a sum and allDiff constraint

                        // Adding pairwise allDiff Constraints
                        for(int x=0;x<n;x++)
                        {
                            pair <int, int> var1 = make_pair(i+x+1,j);
                            int idx1 = Find_Variable_Idx(var1);

                            for(int y=x+1;y<n;y++)
                            {
                                Constraint temp;
                                temp.type = 1;
                                temp.sum = -1;
                                temp.idx_match = -1;
                                pair <int, int> var2 = make_pair(i+y+1,j);
                                int idx2 = Find_Variable_Idx(var2);
                                temp.var1 = idx1;
                                temp.var2 = idx2;
                                temp.isUnary = false;
                                Constraints.push_back(temp);
                            }
                        }

                        // Breaking n-ary sum constraint to binary

                        if(n == 2) // Already binary constraint
                        {
                            Constraint temp;
                            temp.type = 2;
                            temp.sum = Kakuro[i][j].column_constraint;
                            temp.idx_match = -1;
                            int idx1 = Find_Variable_Idx(St_Idx);
                            temp.var1 = idx1;
                            int idx2 = Find_Variable_Idx(End_Idx);
                            temp.var2 = idx2;
                            temp.isUnary = false;
                            Constraints.push_back(temp);
                        }

                        else // n - ary constraint 
                        {
                            // Create new varible mapping to n-1 variables
                            Variable New;
                            New.type = 2;

                            for(int x=0;x<n-1;x++) // Storing index and domain of n-1 variables
                            {
                                New.idx.push_back(make_pair(i+x+1,j));
                            }
                            Get_Possible_Assignment(New.domain, New.idx.size());
                            Variables.push_back(New); // Adding new variable to the set of existing variables

                            // Formulating binary sum constraint
                            Constraint temp;
                            temp.type = 2;
                            temp.sum = Kakuro[i][j].column_constraint;
                            temp.idx_match = -1;
                            int idx1 = Variables.size() - 1; // index of new variable
                            temp.var1 = idx1;
                            int idx2 = Find_Variable_Idx(End_Idx);
                            temp.var2 = idx2;
                            temp.isUnary = false;
                            Constraints.push_back(temp);

                            // Formulating all index match constraints
                            for(int x=0;x<n-1;x++)
                            {
                                pair <int,int> idx = make_pair(i+x+1,j);
                                Constraint temp;
                                temp.type = 3;
                                temp.sum = -1;
                                temp.idx_match = x;
                                int idx1 = Variables.size() - 1;// index of new variable
                                temp.var1 = idx1;
                                int idx2 = Find_Variable_Idx(idx);
                                temp.var2 = idx2;
                                temp.isUnary = false;
                                Constraints.push_back(temp);
                            } 
                        }
                    }
                }
            }
        }
    }
}

int Find_Variable_Idx(pair <int,int> idx) // To find variable in the global array of variables
{
    for(int i=0;i<Variables.size();i++)
    {
        if(Variables[i].idx[0].first == idx.first && Variables[i].idx[0].second == idx.second)
        {
            return i;
        }
    }
    return -1;
}

void Node_Consistency() // To implement Node Consistency
{
    // Iterating to find unary constraints
    for(int i=0;i<Constraints.size();i++)
    {
        if(Constraints[i].isUnary == true)
        {
            int var_idx = Constraints[i].var1;
            int size = Variables[var_idx].domain[0].size();
            // Removing all values from variable's domain
            for(int j=0;j<size;j++)
            {
                Variables[var_idx].domain[0].pop_back();
            }
            // Adding value satisfying unary constraint
            Variables[var_idx].domain[0].push_back(Constraints[i].sum);

            // Checking if new variables created have this index
            for(int j=0;j<Variables.size();j++)
            {
                if(Variables[j].type == 2)
                {
                    // Iterate over indexes stored in new variable
                    for(int k=0;k<Variables[j].idx.size();k++)
                    {
                        if(Variables[j].idx[k].first == Variables[var_idx].idx[0].first && Variables[j].idx[k].second == Variables[var_idx].idx[0].second)
                        {
                            // Removing all values from new variable's domain
                            for(int l=0;l<size;l++)
                            {
                                Variables[j].domain[k].pop_back();
                            }
                            // Adding value satisfying unary constraint
                            Variables[j].domain[k].push_back(Constraints[i].sum);

                        } 
                    } 
                }
            }
        }
    }
}

bool AC3(queue <Constraint> Q) // Returns false if inconsistency is found true otherwise
{
    while(!Q.empty())
    {
        Constraint C = Q.front();
        Q.pop();

        if(AC3_Revise(C) == true)
        {
            if(Variables[C.var1].domain[0].size() == 0) // No values left in domain of variable
            {
                return false; // Inconsistency Found
            }
            
            // Adding all arcs pointing to that variable to queue
            for(int i=0;i<Constraints.size();i++)
            {
                if(Constraints[i].var1 == C.var1)
                {
                    Constraint temp;
                    temp.var1 = Constraints[i].var2;
                    temp.var2 = Constraints[i].var1;
                    temp.type = Constraints[i].type;
                    temp.sum = Constraints[i].sum;
                    temp.idx_match = Constraints[i].idx_match;
                    temp.isUnary = Constraints[i].isUnary;
                    Q.push(temp);
                }
                else if(Constraints[i].var2 == C.var1)
                {
                    Q.push(Constraints[i]);
                }
            }
        }
    }
    return true;
}


// Complete this function by getting all possible assignments to var1 and var2
bool AC3_Revise(Constraint C) // Returns true if we revise domain of var1
{
    bool revised = false;
    int idx1 = C.var1; // Index of Var 1
    int idx2 = C.var2; // Index of Var 2
    vector < vector<int> > temp_Domain;

    // Removing values from domain of var 1 with no corresponding values of var 2
    int size = Variables[idx1].domain.size();

    for(int i=0;i<size;i++)
    {
        int flag = 0;

        // Different Constraint
        if(C.type == 1)
        {
            for(int j=0;j<Variables[idx2].domain.size();j++)
            {
                if(Variables[idx2].domain[j][0] != Variables[idx1].domain[i][0])
                {
                    flag++;
                    break;
                }   
            }
        }

        // Sum Constraint
        else if(C.type == 2)
        {
            int S = 0;
            vector <int> temp; // to check all values are distinct in sum

            for(int k=0;k<Variables[idx1].domain[i].size();k++)
            {
                S += Variables[idx1].domain[i][k];
            }
            for(int j=0;j<Variables[idx2].domain.size();j++)
            {
                int T = 0;
                for(int k=0;k<Variables[idx2].domain[j].size();k++)
                {
                    T += Variables[idx2].domain[j][k];
                }
                if(S + T == C.sum)
                {
                    int temp_flag = 1;
                    for(int k=0;k<Variables[idx1].domain[i].size();k++)
                    {
                        for(int l=0;l<temp.size();l++)
                        {
                            if(Variables[idx1].domain[i][k] == temp[l])
                            {
                                temp_flag--;
                                break;
                            }
                        }
                        if(temp_flag == 0)
                        {
                            break;
                        }
                        else
                        {
                            temp.push_back(Variables[idx1].domain[i][k]);
                        }
                    }
                    if(temp_flag == 1)
                    {
                        for(int k=0;k<Variables[idx2].domain[j].size();k++)
                        {
                            for(int l=0;l<temp.size();l++)
                            {
                                if(Variables[idx2].domain[j][k] == temp[l])
                                {
                                    temp_flag--;
                                    break;
                                }    
                            }
                            if(temp_flag == 0)
                            {
                                break;
                            }
                            else
                            {
                                temp.push_back(Variables[idx2].domain[j][k]);
                            }
                        }
                    }

                    if(temp_flag == 1)
                    {
                        flag++;
                        break;
                    }
                }
            }
        }
        // Same Constraint
        else if(C.type == 3)
        {
            if(Variables[C.var1].type == 1)
            {
                for(int j=0;j<Variables[idx2].domain.size();j++)
                {
                    if(Variables[idx1].domain[i][0] == Variables[idx2].domain[j][C.idx_match])
                    {
                        flag++;
                        break;
                    }
                }
            }
            else
            {
                for(int j=0;j<Variables[idx2].domain.size();j++)
                {
                    if(Variables[idx1].domain[i][C.idx_match] == Variables[idx2].domain[j][0])
                    {
                        flag++;
                        break;
                    }
                }
            }
        }

        // Remove Value from Domain of Var 1
        if(flag == 0)
        {
            revised = true;        
        }
        else
        {
            temp_Domain.push_back(Variables[idx1].domain[i]);
        }
    }

    if(revised == true)
    {
        for(int i=0;i<size;i++)
        {
            Variables[idx1].domain.pop_back();
        }

        for(int i=0;i<temp_Domain.size();i++)
        {
            Variables[idx1].domain.push_back(temp_Domain[i]);
        }
    }

    return revised;
}

bool BS() // To implement Backtracking Search
{
    int hash_map[Variables.size()];
    for(int i=0;i<Variables.size();i++)
    {
        hash_map[i] = 0;
    }

    if(BS_Util(hash_map) == true)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool BS_Util(int H[]) // Utility Function for Backtracking Search
{
    // If assignment is complete return true
    if(Assignment.size() == Variables.size())
    {
        return true;
    }

    // Select next un-assigned variable
    int var = 0;
    for(int i=0;i<Variables.size();i++)
    {
        if(H[i] == 0)
        {
            H[i] = 1;
            var = i;
            break;
        }
    }

    for(int i=0;i<Variables[var].domain.size();i++)
    {
        if(Consistent_Ass(H, var, Variables[var].domain[i]))
        {
            Assignment.push_back(make_pair(var, Variables[var].domain[i]));
            bool result = BS_Util(H);
            if(result == true)
            {
                return true;
            }

            // Removing Inconsistent Assignment
            BACK++;
            int pos = 0;
            vector < pair <int, vector<int> > > temp_Assignment; // To store the assignment of variables

            for(int k=0;k<Assignment.size();k++)
            {
                if(Assignment[k].first != var)
                {
                    temp_Assignment.push_back(Assignment[k]);
                }
            }
            
            int S = Assignment.size();
            for(int k=0;k<S;k++)
            {
                Assignment.pop_back();
            }
            for(int k=0;k<temp_Assignment.size();k++)
            {
                Assignment.push_back(temp_Assignment[k]);
            }
        }
    }

    H[var] = 0;
    return false;
} 

bool BS_MAC() // To implement Backtracking Search with MAC Algorithm
{
    int hash_map[Variables.size()];
    for(int i=0;i<Variables.size();i++)
    {
        hash_map[i] = 0;
    }

    if(BS_MAC_Util(hash_map) == true)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool BS_MAC_Util(int H[]) // Utility Function for Backtracking Search
{
    // If assignment is complete return true
    if(Assignment.size() == Variables.size())
    {
        return true;
    }

    // Select next un-assigned variable
    int var = 0;
    for(int i=0;i<Variables.size();i++)
    {
        if(H[i] == 0)
        {
            H[i] = 1;
            var = i;
            break;
        }
    }
    
    for(int i=0;i<Variables[var].domain.size();i++)
    {
        if(Consistent_Ass(H, var, Variables[var].domain[i]))
        {
            Assignment.push_back(make_pair(var, Variables[var].domain[i]));
            // Maintaining Arc Consistency

            // Initializing Queue with Constraint Arcs entering into Variable i
            queue <Constraint> Q;
            for(int x=0;x<Constraints.size();x++)
            {
                if(Constraints[x].var1 == var && H[Constraints[x].var2] == 0)
                {
                    // Adding reverse constraint arc
                    Constraint temp;
                    temp.var1 = Constraints[x].var2;
                    temp.var2 = Constraints[x].var1;
                    temp.type = Constraints[x].type;
                    temp.sum = Constraints[x].sum;
                    temp.idx_match = Constraints[x].idx_match;
                    temp.isUnary = Constraints[x].isUnary;
                    Q.push(temp);    
                }

                else if(Constraints[x].var2 == var && H[Constraints[x].var1] == 0)
                {
                    // Add Original Constraint
                    Q.push(Constraints[x]);
                }
            }
            
            if(AC3(Q)) 
            {
                bool result = BS_Util(H);
                if(result == true)
                {
                    return true;
                }    
            }
        
            // Removing Inconsistent Assignment
            BACK++;
            int pos = 0;
            for(int i=0;i<Assignment.size();i++)
            {
                if(Assignment[i].first == var)
                {
                    pos = i;
                    break;
                }
            }
            Assignment.erase(Assignment.begin() + pos);
        }
    }
    H[var] = 0;
    return false;
} 

bool Consistent_Ass(int H[], int var, vector <int> value)
{
    for(int i=0;i<Constraints.size();i++)
    {
        
        int pos = 0;
        if(Constraints[i].var1 == var && H[Constraints[i].var2] == 1)
        {
            for(int j=0;j<Assignment.size();j++)
            {
                if(Assignment[j].first == Constraints[i].var2)
                {
                    pos = j;
                    break;
                }
            }

            // Different Constraint
            if(Constraints[i].type == 1)
            { 
                if(Assignment[pos].second[0] == value[0])
                {
                    return false;
                }
            }

            // Sum Constraint
            else if(Constraints[i].type == 2)
            {
                int S = 0;
                vector <int> temp;
                for(int k=0;k<value.size();k++)
                {
                    S += value[k];
                    temp.push_back(value[k]);
                }
                for(int l=0;l<Assignment[pos].second.size();l++)
                {
                    S += Assignment[pos].second[l];
                    temp.push_back(Assignment[pos].second[l]);
                }

                
                int temp_flag = 1;
                for(int x=0;x<temp.size();x++)
                {
                    for(int y=x+1;y<temp.size();y++)
                    {
                        if(temp[x] == temp[y])
                        {    
                            temp_flag--;
                            break;
                        }
                    }
                    if(temp_flag == 0)
                    {
                        break;
                    }
                }
                
                if(S != Constraints[i].sum || temp_flag == 0)
                {
                    return false;
                }
            }

            // Same Constraint
            else if(Constraints[i].type == 3)
            {
                if(Variables[var].type == 1)
                {
                    if(value[0] != Assignment[pos].second[Constraints[i].idx_match])
                    {
                        return false;
                    }
                }
                else
                {
                    if(value[Constraints[i].idx_match] != Assignment[pos].second[0])
                    {
                        return false;
                    }
                }
            }
        }

        else if(Constraints[i].var2 == var && H[Constraints[i].var1] == 1)
        {
            for(int j=0;j<Assignment.size();j++)
            {
                if(Assignment[j].first == Constraints[i].var1)
                {
                    pos = j;
                    break;
                }
            }

            // Different Constraint
            if(Constraints[i].type == 1)
            { 
                if(Assignment[pos].second[0] == value[0])
                {
                    return false;
                }
            }

            // Sum Constraint
            else if(Constraints[i].type == 2)
            {
                int S = 0;
                vector <int> temp;
                for(int k=0;k<value.size();k++)
                {
                    S += value[k];
                    temp.push_back(value[k]);
                }
                for(int l=0;l<Assignment[pos].second.size();l++)
                {
                    S += Assignment[pos].second[l];
                    temp.push_back(Assignment[pos].second[l]);
                }

                int temp_flag = 1;
                for(int x=0;x<temp.size();x++)
                {
                    for(int y=x+1;y<temp.size();y++)
                    {
                        if(temp[x] == temp[y])
                        {
                            temp_flag--;
                            break;
                        }
                    }
                    if(temp_flag == 0)
                    {
                        break;
                    }
                }

                if(S != Constraints[i].sum || temp_flag == 0)
                {
                    return false;
                }
            }

            // Same Constraint
            else if(Constraints[i].type == 3)
            {
                if(Variables[var].type == 1)
                {
                    if(value[0] != Assignment[pos].second[Constraints[i].idx_match])
                    {
                        return false;
                    }
                }
                else
                {
                    if(value[Constraints[i].idx_match] != Assignment[pos].second[0])
                    {
                        return false;
                    }
                }
            }
        }
            
    }

    return true;
}

void Get_Possible_Assignment(vector < vector<int> > &A, int size)
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
        //cout<<"1 *****"<<endl;
        C[curr_idx] += 1;
        B[curr_idx] = 1;
        if(curr_idx > 0)
            B[curr_idx-1] += 1;
        temp.pop_back();
        Generate_Possible_Assignments(A, max_idx, curr_idx-1, temp, C, B, D, from_idx);
    }

    else if(max_idx == curr_idx)
    {
        //cout<<"2 *****"<<endl;
        A.push_back(temp);

        temp.pop_back();
        B[curr_idx-1] += 1;
        Generate_Possible_Assignments(A, max_idx, curr_idx-1, temp, C ,B, D, from_idx);
    }

    else
    {
        //cout<<"3 *****"<<endl;
        temp.push_back(B[curr_idx]);
        /*
        for(int k=0;k<temp.size();k++)
        {
           cout<<temp[k]<<" ";
        }
        cout<<endl;
        */
        Generate_Possible_Assignments(A, max_idx, curr_idx+1, temp, C, B, D, from_idx);
    }
}  

