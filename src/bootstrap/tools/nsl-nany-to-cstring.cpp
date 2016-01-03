#include <yuni/yuni.h>
#include <yuni/io/file.h>
#include <iostream>

using namespace Yuni;



int main(int argc, char** argv)
{
	if (argc != 4)
	{
		std::cerr << "usage: <func> <nany-file> <c-file>\n";
		return 1;
	}

	AnyString funcname{argv[1]};

	Clob nanyfile;
	if (IO::errNone != IO::File::LoadFromFile(nanyfile, argv[2]))
	{
		std::cerr << "failed to load header file from '" << argv[1] << "'\n";
		return 1;
	}

	nanyfile.trim();
	Clob out;
	out.reserve(nanyfile.size() * 2);

	out << "/* !! file generated by cmake */\n";
	out << "#include <stddef.h>\n\n\n";

	out << "const char* " << funcname << "(size_t* size)\n";
	out << "{\n";
	out << "\t*size = " << nanyfile.size() << ";\n";
	out << "\treturn\n";

	String str;
	nanyfile.words("\r\n", [&](const AnyString& line) -> bool {
		str = line;
		str.replace("\"", "\\\"");
		out << "\t\t\"" << str << "\\n\" \\\n";
		return true;
	});
	out << "\t\t;\n";

	out << "}\n";

	if (not IO::File::SetContent(argv[3], out))
	{
		std::cerr << "failed to write '" << argv[3] << "'\n";
		return 1;
	}

	return 0;
}
