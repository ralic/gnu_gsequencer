#include <glib.h>
#include <glib-object.h>

#include <midi2ags/midi/ags_midi_parser.h>

#include <stdio.h>

int
main(int argc, char **argv)
{
  AgsMidiParser *midi_parser;

  xmlDoc *doc;
  FILE *out;
  
  xmlChar *buffer;
  gchar *filename;
  size_t length;
  int fd;
  
  if(argc == 2){
    filename = argv[1];
  }else{
    return;
  }

  fd = fopen(filename, "r\0");
  fseek(fd, 0, SEEK_SET);
  midi_parser = ags_midi_parser_new(fd);

  doc = ags_midi_parser_parse_full(midi_parser);
  
  xmlDocDumpFormatMemoryEnc(doc, &(buffer), &length, "UTF-8", TRUE);

  fwrite(buffer, length, sizeof(xmlChar), stdout);
  fflush(stdout);

  
  return(0);
}