#ifndef SCOOBLE_SEARCH_MIME_H
#define SCOOBLE_SEARCH_MIME_H

struct MimetypeMap
   {
      const char *Extension, *Mimetype;
   };

const MimetypeMap MimeTable[] = {
   // List of some of the most common MIME types in sorted order.
   // https://developer.mozilla.org/en-US/docs/Web/HTTP/Basics_of_HTTP/MIME_types/Complete_list_of_MIME_types
   ".3g2",
   "video/3gpp2",
   ".3gp",
   "video/3gpp",
   ".7z",
   "application/x-7z-compressed",
   ".aac",
   "audio/aac",
   ".abw",
   "application/x-abiword",
   ".arc",
   "application/octet-stream",
   ".avi",
   "video/x-msvideo",
   ".azw",
   "application/vnd.amazon.ebook",
   ".bin",
   "application/octet-stream",
   ".bz",
   "application/x-bzip",
   ".bz2",
   "application/x-bzip2",
   ".csh",
   "application/x-csh",
   ".css",
   "text/css",
   ".csv",
   "text/csv",
   ".doc",
   "application/msword",
   ".docx",
   "application/vnd.openxmlformats-officedocument.wordprocessingml.document",
   ".eot",
   "application/vnd.ms-fontobject",
   ".epub",
   "application/epub+zip",
   ".gif",
   "image/gif",
   ".htm",
   "text/html",
   ".html",
   "text/html",
   ".ico",
   "image/x-icon",
   ".ics",
   "text/calendar",
   ".jar",
   "application/java-archive",
   ".jpeg",
   "image/jpeg",
   ".jpg",
   "image/jpeg",
   ".js",
   "application/javascript",
   ".json",
   "application/json",
   ".mid",
   "audio/midi",
   ".midi",
   "audio/midi",
   ".mpeg",
   "video/mpeg",
   ".mpkg",
   "application/vnd.apple.installer+xml",
   ".odp",
   "application/vnd.oasis.opendocument.presentation",
   ".ods",
   "application/vnd.oasis.opendocument.spreadsheet",
   ".odt",
   "application/vnd.oasis.opendocument.text",
   ".oga",
   "audio/ogg",
   ".ogv",
   "video/ogg",
   ".ogx",
   "application/ogg",
   ".otf",
   "font/otf",
   ".pdf",
   "application/pdf",
   ".png",
   "image/png",
   ".ppt",
   "application/vnd.ms-powerpoint",
   ".pptx",
   "application/vnd.openxmlformats-officedocument.presentationml.presentation",
   ".rar",
   "application/x-rar-compressed",
   ".rtf",
   "application/rtf",
   ".sh",
   "application/x-sh",
   ".svg",
   "image/svg+xml",
   ".swf",
   "application/x-shockwave-flash",
   ".tar",
   "application/x-tar",
   ".tif",
   "image/tiff",
   ".tiff",
   "image/tiff",
   ".ts",
   "application/typescript",
   ".ttf",
   "font/ttf",
   ".vsd",
   "application/vnd.visio",
   ".wav",
   "audio/x-wav",
   ".weba",
   "audio/webm",
   ".webm",
   "video/webm",
   ".webp",
   "image/webp",
   ".woff",
   "font/woff",
   ".woff2",
   "font/woff2",
   ".xhtml",
   "application/xhtml+xml",
   ".xls",
   "application/vnd.ms-excel",
   ".xlsx",
   "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet",
   ".xml",
   "application/xml",
   ".xul",
   "application/vnd.mozilla.xul+xml",
   ".zip",
   "application/zip"
};

//  Multipurpose Internet Mail Extensions (MIME) types
const char *getMimetype( const std::string path )
   {
   std::string fnPath( path );
   int extensionStart = fnPath.find_last_of( '.' );
   const char *ext = fnPath.substr( extensionStart ).c_str();

   for ( int i = 0;  i < sizeof( MimeTable );  i++ )
      {
      if ( !strcmp( MimeTable[ i ].Extension, ext ) )
         {
         return MimeTable[ i ].Mimetype;
         }
      }

   // Anything not matched is an "octet-stream", treated
   // as an unknown binary, which can be downloaded.
   return "application/octet-stream";
   }
#endif // SCOOBLE_SEARCH_MIME_H
