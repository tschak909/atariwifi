#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <iomanip>
#include <iostream>
#include <fstream>
//using namespace std;

#define EOL 155
#define EOLS -101

// todo:
// specify: page size, left/bottom margins, line spacing
// sio simulation: read input file, make 40-char buffers, replace CRLF with EOL, pad with spaces to 40 char
// using simulated sio buffer to create text stream payload for PDF lines
// use standard C and POSIX file output (don't care about input because that's SIO simulation)
// replace page(string) with incremental line string and accumulate lengths for pdf_xref
// use NUMLINES to parameterize the routine

std::ifstream prtin; // input file

FILE *f; // standard C output file

bool BOLflag = true;
float svg_X = 0.;
float svg_Y = 0.;
float svg_X_home = 0.;
float svg_Y_home = 0.;
float svg_text_y_offset = 0.;
float pageWidth = 550.;
float printWidth = 480.;
double leftMargin = 0.0;
float charWidth = 12.;
float lineHeight = 20.8;
float fontSize = 20.8;
int svg_color_idx = 0;
std::string svg_colors[4] = {"Black", "Blue", "Green", "Red"};
int svg_line_type = 0;
int svg_arg[3] = {0, 0, 0};

bool escMode = false;
bool escResidual = false;
bool textMode = true;
bool svg_home_flag = true;

void svg_new_line()
{
  // http://scruss.com/blog/2016/04/23/fifteentwenty-commodore-1520-plotter-font/
  // <text x="0" y="15" fill="red">I love SVG!</text>
  // position new line and start text string array
  //fprintf(f,"<text x=\"0\" y=\"%g\" font-size=\"%g\" font-family=\"ATARI 1020 VECTOR FONT APPROXIM\" fill=\"black\">", svg_Y,fontSize);
  if (svg_home_flag)
  {
    svg_text_y_offset = -lineHeight;
    svg_home_flag = false;
  }
  svg_Y += lineHeight;
  svg_X = 0; // always start at left margin? not sure of behavior
  fprintf(f, "<text x=\"%g\" y=\"%g\" font-size=\"%g\" font-family=\"FifteenTwenty\" fill=\"%s\">", leftMargin, svg_Y + svg_text_y_offset, fontSize, svg_colors[svg_color_idx].c_str());

  BOLflag = false;
}

void svg_end_line()
{
  // <text x="0" y="15" fill="red">I love SVG!</text>
  fprintf(f, "</text>\n"); // close the line
  BOLflag = true;
}

void svg_plot_line()
{
  //<line x1="0" x2="100" y1="0" y2="100" style="stroke:rgb(0,0,0);stroke-width:2 />
  int x1 = svg_X_home + svg_X;
  int y1 = svg_Y_home + svg_Y;
  int x2 = x1 + svg_arg[0];
  int y2 = y1 + svg_arg[1];
  svg_X += svg_arg[0];
  svg_Y += svg_arg[1];

  fprintf(f, "<line x1=\"%d\" x2=\"%d\" y1=\"%d\" y2=\"%d\" style=\"stroke:%s;stroke-width:2\" />\n", x1, x2, y1, y2, svg_colors[svg_color_idx].c_str());
}

/*
https://www.atarimagazines.com/v4n10/atari1020plotter.html

INSTRUCTION		FORM			        MODE

GRAPHICS		  ESC ESC CTRL G		-
TEXT			    DEFAULT			      AT CHANNEL OPENING
TEXT			    A			            TEXT FROM GR.
20 COL. TEXT	ESC ESC CTRL P		TEXT
40 COL. TEXT	ESC ESC CTRL N		TEXT
80 COL. TEXT	ESC ESC CTRL S		TEXT
HOME			    H			            GRAPHICS
PEN COLOR		  C (VALUE 0-3)		  GRAPHICS
0	Black
1	Blue 
2	Green
3	Red  
LINE TYPE		  L (VALUE 1-15)		GRAPHICS
0=SOLID		  	-		            	-
DRAW			    DX,Y			        GRAPHICS
MOVE			    MX,Y			        GRAPHICS
ROTATE TEXT		Q(0-3)			      GRAPHICS
(Text to be rotated must start with P)
PRINT TEXT    P(any text)       GRAPHICS
INITIALIZE		I			            GRAPHICS
(Sets current X,Y as HOME or 0,0)
RELATIVE DRAW	JX,Y			        GRAPHICS
(Used with Init.)
RELATIVE MOVE	RX,Y			        GRAPHICS
(Used with Init.)
CHAR. SCALE		S(0-63)			      GRAPHICS
*/

void svg_get_arg(std::string S, int n)
{
  svg_arg[n] = atoi(S.c_str());
  std::cout << "arg " << n << ": " << svg_arg[n] << ", ";
}

void svg_get_2_args(std::string S)
{
  size_t n = S.find_first_of(',');
  svg_get_arg(S.substr(0, n), 0);
  svg_get_arg(S.substr(n + 1), 1);
}

void svg_get_3_args(std::string S)
{
  size_t n1 = S.find_first_of(',');
  size_t n2 = S.find_first_of(',', n1 + 1);
  svg_get_arg(S.substr(0, n1 - 1), 0);
  svg_get_arg(S.substr(n1 + 1, n2 - n1), 1);
  svg_get_arg(S.substr(n2 + 1), 2);
}

void svg_graphics_command(std::string S)
{
  // parse the graphics command
  // graphics mode commands start with a single LETTER
  // commands have 0, 1 or 2 arg's
  // - or X has 3 args and "P" is followed by a string
  // commands are terminated with a "*"" or EOL
  // a ";" after the args repeats the command CHAR
  //
  // could maybe use regex but going to brute force with a bunch of cases

  size_t cmd_pos = 0;
  do
  {
    char c = S[cmd_pos];
    switch (c)
    {
    case 'A':
      textMode = true;
      break;
    case 'H':
      svg_X = svg_X_home;
      svg_Y = svg_Y_home;
      break;
    case 'C':
      // get arg out of S and assign to...
      svg_get_arg(S.substr(cmd_pos+1), 0);
      svg_color_idx = svg_arg[0];
      break;
    case 'L':
      // get arg out of S and assign to...
      svg_get_arg(S.substr(cmd_pos+1), 0);
      svg_line_type = svg_arg[0];
      break;
    case 'D':
      // get 2 args out of S and draw a line
      svg_get_2_args(S.substr(cmd_pos+1));
      svg_plot_line();
      break;
    case 'M':
      // get 2 args out of S and ...
      svg_get_2_args(S.substr(cmd_pos+1));
      svg_X = svg_X_home + svg_arg[0];
      svg_Y = svg_Y_home + svg_arg[1];
      break;
    case 'I':
      svg_X_home = svg_X;
      svg_Y_home = svg_Y;
      svg_home_flag = true;
      break;
    default:
      return;
    }
    size_t new_pos = S.find_first_of(":*", cmd_pos + 1);
    if (new_pos == std::string::npos)
      return;
    if (S[new_pos] == ':')
      S[new_pos] = S[cmd_pos];
    else if (S[new_pos] == '*')
      new_pos++;
    cmd_pos = new_pos;
  } while (true);
}

void svg_handle_char(unsigned char c)
{
  if (escMode)
  {
    // Atari 1020 escape codes:
    // ESC CTRL-G - graphics mode (simple A returns)
    // ESC CTRL-P - 20 char mode
    // ESC CTRL-N - 40 char mode
    // ESC CTRL-S - 80 char mode
    // ESC CTRL-W - international char set
    // ESC CTRL-X - standard char set
    if (c == 7)
    {
      textMode = false;
      std::cout << "entering GRAPHICS mode!\n";
      escMode = false;
      return;
    }
    if (c == 16)
    {
      charWidth = 24.;
      fontSize = 2. * 20.8;
      lineHeight = fontSize;
    }
    if (c == 14)
    {
      charWidth = 12.;
      fontSize = 20.8;
      lineHeight = fontSize;
    }
    if (c == 19)
    {
      charWidth = 6.;
      fontSize = 10.4;
      lineHeight = fontSize;
    }
    escMode = false;
    escResidual = true;
  }
  else if (c == 27)
    escMode = true;
  else if (c > 31 && c < 127) // simple ASCII printer
  {
    // what characters need to be escaped in SVG text?
    // if (c == BACKSLASH || c == LEFTPAREN || c == RIGHTPAREN)
    //   _file->write(BACKSLASH);
    fputc(c, f);        //_file->write(c);
    svg_X += charWidth; // update x position
    //std::cout << svg_X << " ";
  }
}

void svg_header()
{
  // <!DOCTYPE html>
  // <html>
  // <body>
  // <svg height="210" width="500">

  //fprintf(f,"<!DOCTYPE html>\n");
  //fprintf(f,"<html>\n");
  //fprintf(f,"<body>\n\n");
  fprintf(f, "<svg height=\"2000\" width=\"480\" style=\"background-color:white\" viewBox=\"0 -1000 480 2000\">\n");
  svg_home_flag = true;
}

void svg_footer()
{
  // </svg>
  // </body>
  // </html>

  if (!BOLflag)
    svg_end_line();
  fprintf(f, "</svg>\n\n");
  //fprintf(f,"</body>\n");
  //fprintf(f,"</html>\n");
}

void svg_add(std::string S)
{
  // looks like escape codes take you out of GRAPHICS MODE
  if (S[0] == 27)
    textMode = true;
  if (!textMode)
    svg_graphics_command(S);
  else
  {
    // loop through string
    for (int i = 0; i < S.length(); i++)
    {
      unsigned char c = (unsigned char)S[i];
      //std::cout << "c=" << c << " ";

      if (escResidual)
      {
        escResidual = false;
        if (c == EOL)
          return;
      }
      // the following creates a blank line of text
      if (BOLflag && c == EOL)
      { // svg_new_line();
        svg_Y += lineHeight;
        return;
      }
      // check for EOL or if at end of line and need automatic CR
      if (!BOLflag && c == EOL)
      {
        svg_end_line();
        return;
      }
      else if (!BOLflag && (svg_X > (printWidth - charWidth)))
      {
        svg_end_line();
        svg_new_line();
      } // or do I just need to start a new line of text
      else if (BOLflag && c != 27 && !escMode)
        svg_new_line();
      // disposition the current byte
      svg_handle_char(c);
      if (!textMode)
        return;
    }
  }
}

int main()
{

  std::string payload;

  int j;

  prtin.open("in.txt");
  if (prtin.fail())
    return -1;
  f = fopen("out.svg", "w");

  svg_header();
  //SIMULATE SIO:
  //standard Atari P: handler sends 40 bytes at a time
  //break up line into 40-byte buffers
  do
  {
    payload.clear();
    char c = '\0';
    int i = 0;
    while (c != EOLS && i < 40 && !prtin.eof())
    {
      prtin.get(c);
      if (c == '\n')
        c = EOLS;
      payload.push_back(c);
      i++;
    }
    std::cout << payload << ", ";
    svg_add(payload);
    fflush(f);
  } while (!prtin.eof());
  svg_footer();
  fclose(f);
  return 0;
}
