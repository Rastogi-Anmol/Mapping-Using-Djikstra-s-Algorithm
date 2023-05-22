/*
---------------------------------------------------
# Name : Anmol Rastogi
# SID: 1733365
# CCID : arastog1
# AnonID : 1000340771
# CMPUT 275 , Winter 2023
#
# Assignment Part 2: Client/Server Application
----------------------------------------------------
*/
#include <iostream>
#include <cassert>
#include <fstream>
#include <string>
#include <list>

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <cstring>

#include "wdigraph.h"
#include "dijkstra.h"
//defining a constant
#define _MSG_MAX_LENGTH 100


struct Point {
    long long lat, lon;
};


// return the manhattan distance between the two points
long long manhattan(const Point& pt1, const Point& pt2) {
  long long dLat = pt1.lat - pt2.lat, dLon = pt1.lon - pt2.lon;
  return abs(dLat) + abs(dLon);
}


// find the ID of the point that is closest to the given point "pt"
int findClosest(const Point& pt, const unordered_map<int, Point>& points) {
  pair<int, Point> best = *points.begin();

  for (const auto& check : points) {
    if (manhattan(pt, check.second) < manhattan(pt, best.second)) {
      best = check;
    }
  }
  return best.first;
}


// read the graph from the file that has the same format as the "Edmonton graph" file
void readGraph(const string& filename, WDigraph& g, unordered_map<int, Point>& points) {
  ifstream fin(filename);
  string line;

  while (getline(fin, line)) {
    // split the string around the commas, there will be 4 substrings either way
    string p[4];
    int at = 0;
    for (auto c : line) {
      if (c == ',') {
        // start new string
        ++at;
      }
      else {
        // append character to the string we are building
        p[at] += c;
      }
    }

    if (at != 3) {
      // empty line
      break;
    }

    if (p[0] == "V") {
      // new Point
      int id = stoi(p[1]);
      assert(id == stoll(p[1])); // sanity check: asserts if some id is not 32-bit
      points[id].lat = static_cast<long long>(stod(p[2])*100000);
      points[id].lon = static_cast<long long>(stod(p[3])*100000);
      g.addVertex(id);
    }
    else {
      // new directed edge
      int u = stoi(p[1]), v = stoi(p[2]);
      g.addEdge(u, v, manhattan(points[u], points[v]));
    }
  }
}


int create_and_open_fifo(const char * pname, int mode) {
  // create a fifo special file in the current working directory with
  // read-write permissions for communication with the plotter app
  // both proecsses must open the fifo before they perform I/O operations
  // Note: pipe can't be created in a directory shared between 
  // the host OS and VM. Move your code outside the shared directory
  if (mkfifo(pname, 0666) == -1) {
    cout << "Unable to make a fifo. Make sure the pipe does not exist already!" << endl;
    cout << errno << ": " << strerror(errno) << endl << flush;
    exit(-1);
  }

  // opening the fifo for read-only or write-only access
  // a file descriptor that refers to the open file description is
  // returned
  int fd = open(pname, mode);

  if (fd == -1) {
    cout << "Error: failed on opening named pipe." << endl;
    cout << errno << ": " << strerror(errno) << endl << flush;
    exit(-1);
  }
  return fd;
}


/*
Description: Runs loop to take input from the inpipe line wise
             and returns the input as a string.

Arguments:
          in(int&): The inpipe open integer
          msg[_MSG_MAX_LENGTH](char): A character array storing the message line
          idx(int): starting index of the line
          stop_char(char): character when the line would end

Return: (String) The input form the client converted to a string
*/
string take_input_char(int& in ,char msg[_MSG_MAX_LENGTH], int idx = 0, char stop_char = '\n')
{
  char current; 
  while(true)
  {
  read(in,&current,1);
  if(current == stop_char)
  {
    break;
  }
  msg[idx++] = current;
  }
  string return_string = msg;
  return return_string;
}


/*
Description: Gets the latitutde and longitude of the
             given Point from the values in the string

Arguments:
          s(string): The input string fromt the client
          sPoint(Point&) : The start point of the path

Return: None
*/
void get_points(string s,Point& spoint)
{
  for(int i = 0 ; i < _MSG_MAX_LENGTH ; i ++)
  {
    if(s[i] == ' ')
    {
      spoint.lat = static_cast<long long>((stod(s.substr(0,i)))*100000);
      spoint.lon = static_cast<long long>((stod(s.substr(i+1,s.size()-i-1)))*100000);
      break;
    }
  }
}


/*
Description: Checks if the client has enetered Q to quit the program.
             If takes input and finds the start vertec and 
             end vertex

Arguments:
          in(int&): The inpipe open integer
          out(int&): The outpipe open integer
          sPoint(Point&) : The start point of the path
          ePoint(Point&) : The end point of the path
          inpipe(const char*): pointer to the inpipe
          outpipe(const char*): pointer to the outpipe

Return: None
*/
void take_input(int& in,int& out,Point& sPoint,Point& ePoint,const char *inpipe,const char *outpipe)
{
  char msg1[_MSG_MAX_LENGTH],msg2[_MSG_MAX_LENGTH],current; 
  string input1 ,input2;
  //reads the first character to check if the client has called 'Q'
  read(in,&current,1);
  msg1[0] = current;
  if(msg1[0] != 'Q')
  {
    //takes input from the user 
    input1 = take_input_char(in,msg1,1);
    input2 = take_input_char(in,msg2);
    //converts input into start and end vertex
    get_points(input1,sPoint);
    get_points(input2,ePoint);
  }
  else
  {
    //closes and unlinks pipes
    close(in);
    close(out);
    unlink(inpipe);
    unlink(outpipe);
    //exits the program
    exit(0);
  }
}


int main() 
{
  WDigraph graph;
  unordered_map<int, Point> points;

  const char *inpipe = "inpipe";
  const char *outpipe = "outpipe";

  // Open the two pipes 
  int in = create_and_open_fifo(inpipe, O_RDONLY);
  cout << "inpipe opened..." << endl;
  int out = create_and_open_fifo(outpipe, O_WRONLY);
  cout << "outpipe opened..." << endl;  

  // build the graph
  readGraph("server/edmonton-roads-2.0.1.txt", graph, points);

  //runs till the exit is called explicitly 
  while(true)
  {
    Point sPoint,ePoint;
    take_input(in,out,sPoint,ePoint,inpipe,outpipe);
    // read a request

    // get the points closest to the two points we read
    int start = findClosest(sPoint, points), end = findClosest(ePoint, points);

    // run dijkstra's algorithm, this is the unoptimized version that
    // does not stop when the end is reached but it is still fast enough
    unordered_map<int, PIL> tree;
    dijkstra(graph, start, tree);


    char new_line = '\n';
    char end_of_route = 'E';
    // if there is a path
    if (tree.find(end) != tree.end()) 
    {
      // read off the path by stepping back through the search tree
      list<int> path;
      while (end != start) 
      {
        path.push_front(end);
        end = tree[end].first;
      }
      path.push_front(start);
      //reads from the path tree and writes it into the outpipe
      for (int v : path) 
      {
        string output_line;
        //converts the long long latitude and logitude to double and writes into the 
        //outpipe after dividing by 10^5.
        double longitude = static_cast<double>(points[v].lon);
        double latitude  = static_cast<double>(points[v].lat); 
        string longitude_string = to_string(latitude/100000);
        longitude_string.pop_back();
        //remvoes the last zero from the string
        string latitude_string = to_string(longitude/100000);
        latitude_string.pop_back();
        output_line = longitude_string + " " + latitude_string;
        for( char c : output_line ) 
        {
          write(out, &c, 1);
        }
        write(out,&new_line,1);
      }
    }
    //writes E at the end in the outpipe
    write(out,&end_of_route,1);
    write(out,&new_line,1);
  }
  return 0;
}
