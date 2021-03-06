#include <asl/Array.h>
#include <asl/Map.h>
#include <asl/Var.h>
#include <asl/Xdl.h>
#include <asl/CmdArgs.h>
#include <asl/TabularDataFile.h>
#include <asl/IniFile.h>
#include <asl/File.h>
#include <asl/TextFile.h>
#include <asl/util.h>
#include <stdio.h>

void testFactory();
void testHashMap();
void testMap();
void testStaticSpace();
void testPath();
void testAtomicCount();
void testXML();
void testProcess();
void testSHA1();
void testSmartObject();
void testDate();
void testVec3();
void testUuid();

using namespace asl;


void testFile()
{
	File file("c:/dir1/dir.2/file.ext");
	ASL_ASSERT(file.directory() == "c:/dir1/dir.2");
	ASL_ASSERT(file.name() == "file.ext");
	ASL_ASSERT(file.extension() == "ext");
	ASL_ASSERT(file.hasExtension("txt|EXT"));
	
	TextFile tfile("lines.txt", File::WRITE);
	String line1 = "123";
	String line2 = String('x', 4000);
	tfile << line1 << '\n';
	tfile << line2 << '\n';
	tfile.close();

	Array<String> lines = TextFile("lines.txt").lines();
	ASL_ASSERT(lines[0] == line1);
	ASL_ASSERT(lines[1] == line2);
}

void testIniFile()
{
	{
		IniFile file("config.ini");
		file["global"] = "global value";
		file["sec1/field1"] = "value1";
		file["sec1/field2"] = "value2";
		file["sec2/field"] = "value3";

		file["list/1\\x"] = "7";
		file["list/2\\y"] = "3";
		file["list/size"] = "2";
	}
	{
		IniFile file("config.ini");
		ASL_ASSERT(file.sections.length() == 4);
		ASL_ASSERT(file.sections["sec1"].vars().length() == 2);
		ASL_ASSERT(file["global"] == "global value");
		ASL_ASSERT(file["sec1/field1"] == "value1");
		ASL_ASSERT(file["sec1/field2"] == "value2");
		ASL_ASSERT(file["sec2/field"] == "value3");
		
		ASL_ASSERT(file.arraysize("list") == 2);
		ASL_ASSERT(file.array("x", 0) == "7");
		ASL_ASSERT(file.array("y", 1) == "3");
	}
}

void testTabularDataFile()
{
	int N = 100;
	
	{
		TabularDataFile file("data.csv");
		file.flushEvery(1);
		file.useQuotes();
		//file.setSeparator(';');
		//file.setDecimal(',');

		file.columns("i,x,y,sign:neg|pos");

		if(!file.ok())
		{
			printf("Can't write file\n");
			ASL_ASSERT(0);
		}

		for(int i=0; i<N; i++)
		{
			file << i << 0.5 << -3.0*i << "neg";
		}
	}

	{
		TabularDataFile file("data.arff");

		file.columns("i,x,y,sign:neg|pos");
		file.flushEvery(100);

		if(!file.ok())
		{
			printf("Can't write file\n");
			ASL_ASSERT(0);
		}

		for(int i=0; i<N; i++)
		{
			file << i << 0.5 << -3.0*i << "neg";
		}
	}

	{
		//double t1 = now();
		TabularDataFile file("data.csv");
		int k = 0;
		while(file.nextRow())
		{
			ASL_ASSERT(file[0].is(Var::NUMBER) && file[1].is(Var::NUMBER) && file[2].is(Var::NUMBER) && file[3].is(Var::STRING));
			int i = file[0];
			double x = file[1];
			double y = file[2];
			String s = file[3];
			ASL_ASSERT(i == k++);
			ASL_ASSERT(x == 0.5);
			ASL_ASSERT(y == -3.0*i);
			ASL_ASSERT(s == "neg");
			ASL_ASSERT(file[0] == file["i"] && file[1] == file["x"] && file[2] == file["y"] && file[3] == file["sign"]);
		}
		//double t2 = now();
	}
}

void testCmdArgs()
{
	Array<const char*> argv;
	argv << "convert" << "-format" << "jpeg" << "-fast" << "-q" << "85" <<
		"-k" << "k1" << "-k" << "k2" << "-gray" << "on" << "-rgb" << "no" << "-progressive!" << "-scale" << "-1.0" << "image1.png" << "image2.bmp";
	//const char* argvv[] = {"convert" , "-format" , "jpeg" , "-fast" , "-q" , "85" ,
	//	"-k" , "k1" , "-k" , "k2" , "-gray" , "on" , "-rgb" , "no", "-progressive!", "image1.png" , "image2.bmp"};

	CmdArgs args(argv.length(), (char**)argv.ptr());

	ASL_ASSERT( args.has("format") );
	ASL_ASSERT( !args["size"] );
	ASL_ASSERT( !args.has("size") );
	ASL_ASSERT( args["format"] == "jpeg" );
	ASL_ASSERT( args.has("fast") );
	ASL_ASSERT( (int)args("q", 99) == 85 );
	ASL_ASSERT( (int)args("Q", 99) == 99 );
	ASL_ASSERT( args["k"] == "k2" );
	ASL_ASSERT( args("k").length() == 2 );
	ASL_ASSERT( args("k")[0] == "k1" );
	ASL_ASSERT( args.is("gray") );
	ASL_ASSERT( !args.is("rgb") );
	ASL_ASSERT( !args.is("progrssive") );
	ASL_ASSERT( args.is("progressive"));
	ASL_ASSERT( args.all().length() == 19 );
	ASL_ASSERT( args.length() == 2 );
	ASL_ASSERT( args[0] == "image1.png" );
	ASL_ASSERT( args[1] == "image2.bmp" );
	ASL_ASSERT( ((double)args["scale"] - -1.0) < 1e-10);
	ASL_ASSERT( args.untested().length() == 0);

	const char* argv2[] = {"convert" , "-format" , "jpeg" , "-q" , "85", "-fast", "image1.bmp"};

	CmdArgs args2(7, (char**)argv2, "format:,q:,fast");

	ASL_ASSERT( args2.is("fast") );
	ASL_ASSERT( args2["format"] == "jpeg" );
	ASL_ASSERT( args2[0] == "image1.bmp" );
	ASL_ASSERT( args2.length() == 1 );
}

String join(const Array<String>& a)
{
	return a.join("-");
}

void testArray()
{
	Array<int> a;
	a << 3 << -5 << 10 << 0;

	ASL_ASSERT(a.length() == 4);
	ASL_ASSERT(a[0] == 3 && a[1] == -5 && a[2] == 10 && a[3] == 0);
	ASL_ASSERT(a == array(3, -5, 10, 0) );

	ASL_ASSERT(a.last() == 0);

	ASL_ASSERT(a.contains(-5));
	ASL_ASSERT(!a.contains(22));

	ASL_ASSERT(a.indexOf(10) == 2);

	a.sort();

	ASL_ASSERT(a[0] == -5 && a[1] == 0 && a[2] == 3 && a[3] == 10);

	Array<String> names;
	names << "Alvaro" << "Segura";

	ASL_ASSERT( names.join(",") == "Alvaro,Segura" );

#ifdef ASL_HAVE_INITLIST
	Array<int> c = { 1, 2 };
	ASL_ASSERT(c.length() == 2);
	ASL_ASSERT(c[0] == 1 && c[1] == 2);

	// This fails in gcc 4.4.4 !!
	ASL_ASSERT(join({ "a", "b" }) == "a-b");
#endif

	Array<int> b = array(5, 3, -1, 2, 10, 7);
	int s = 0;

	foreach(int x, b)
	{
		if (x < 0)
			continue;
		if (x > 9)
			break;
		s += x;
	}
	ASL_ASSERT(s == 10);
	s = 0;
	foreach2(int i, int x, b)
	{
		if (i < 2)
			continue;
		if (i > 3)
			break;
		s += i * x;
	}
	ASL_ASSERT(s == -1 * 2 + 2 * 3);
}


void testString()
{
	String xxx = String::repeat('x', 1000);
	ASL_ASSERT(xxx.length() == 1000);
	for(int i=0; i<1000; i++)
		ASL_ASSERT(xxx[i] == 'x');
	String f1(15, "%s", *xxx);

	ASL_ASSERT(f1 == xxx);

	String u1(L"1");

	ASL_ASSERT(u1 == "1");

	String a = "a";
	String b = 123;
	String c = 'c';
	ASL_ASSERT(a+b+c == "a123c");
	ASL_ASSERT("a" + b + 'c' == "a123c");

	String n = -65536;

	ASL_ASSERT(n == "-65536");
	
	String d = "My taylor is rich";
	ASL_ASSERT(d.startsWith("My"));
	ASL_ASSERT(d.endsWith("rich"));
	ASL_ASSERT(!d.endsWith("poor"));
	ASL_ASSERT(d.contains("taylor"));
	ASL_ASSERT(!d.contains("doctor"));
	
	ASL_ASSERT(d.substring(3) == "taylor is rich");
	ASL_ASSERT(d.substring(3, 6) == "tay");
	ASL_ASSERT(d.substr(-3) == "ich");
	ASL_ASSERT(d.substr(-4, 3) == "ric");
	ASL_ASSERT(d.substr(0, 30) == d);
	
	Array<String> w = d.split(' ');
	ASL_ASSERT(w.length() == 4);
	ASL_ASSERT(w.last() == "rich");
	ASL_ASSERT(w.join("--") == "My--taylor--is--rich");

	ASL_ASSERT(String(" \t troll\r \n ").trimmed() == "troll");

	String untrimmed = " \t troll\r \n ";
	untrimmed.trim();
	printf("[%s]\n", *untrimmed);
	ASL_ASSERT(untrimmed == "troll");

	String e;

	e << -3 << 0.5 << "ab" << 'c';

	ASL_ASSERT(e == "-30.5abc");

	String f = "My taylor is a taylor";

	ASL_ASSERT(f.lastIndexOf("taylor") == 15);

	String h = "3eB0";
	ASL_ASSERT(h.hexToInt() == 0x3eb0);

#ifdef ASL_ANSI
	String g = "My tailor is rich 1990";
	ASL_ASSERT(g.toUpperCase() == "MY TAILOR IS RICH 1990");
	ASL_ASSERT(g.equalsNocase("My Tailor is Rich 1990"));
	ASL_ASSERT(!g.equalsNocase("My Taylor is Rich 1990"));
#else
	String g = "Ñandú εξέλιξη жизни";
	ASL_ASSERT(g.toUpperCase() == "ÑANDÚ ΕΞΈΛΙΞΗ ЖИЗНИ");
	ASL_ASSERT(g.equalsNocase("ñanDÚ εΞΈλΙξΗ ЖиЗНИ"));
	ASL_ASSERT(!g.equalsNocase("ñanDU εΞΈλΙξΗ ЖиЗНИ"));
	String unicode = "añ€😀";
	wchar_t wunicode[16];
	utf8toUtf16(unicode, wunicode, 15);
	Array<int> chars = unicode.chars();
	ASL_ASSERT(chars.length() == 4 && chars[0] == 97 && chars[1] == 241 && chars[2] == 0x20ac && chars[3] == 0x1f600);
	String unicode2 = wunicode;
	ASL_ASSERT(unicode2 == unicode);
#endif
	ASL_ASSERT(String(" \rmy  taylor\n\tis rich\r\n").split().join('_') == "my_taylor_is_rich");
	ASL_ASSERT(String("my  taylor is rich").split().join('_') == "my_taylor_is_rich");

	String empty = "";
	ASL_ASSERT(!empty);
	ASL_ASSERT(String("c"));
	String full = "abc";
	int valid1 = empty? 1 : 0;
	int valid2 = full? 1 : 0;
	ASL_ASSERT(valid1 == 0 && valid2 == 1);

	ASL_ASSERT(!empty.isTrue());
	ASL_ASSERT(!String("false").isTrue());
}

void testXDL()
{
	String a = "A/*...*/{x=3.5, //...\ny=\"s\", z=[Y, N]}";
	Var b = decodeXDL(a);
	ASL_ASSERT(b.is("A"));
	ASL_ASSERT(b["x"].is(Var::NUMBER) && fabs((double)b["x"] - 3.5) < .0000001);
	ASL_ASSERT(b["y"]=="s");
	ASL_ASSERT(b["z"].is(Var::ARRAY));
	ASL_ASSERT(b["z"].length() == 2);
	ASL_ASSERT(b["z"][0] == true);
	ASL_ASSERT(b["z"][1] == false);
	String c = encodeXDL(b);
	ASL_ASSERT(c == "A{x=3.5,y=\"s\",z=[Y,N]}");
	String d = "A/*...*/{x=3.5, //...\ny=\"s\", z=[Y, N)}";
	Var e = decodeXDL(d);
	ASL_ASSERT(e.is(Var::NONE));
	Var f = decodeJSON("{\"x\":null,\"y\":3}");
	ASL_ASSERT(!f.is(Var::NONE));
	ASL_ASSERT(f["y"] == 3);
	ASL_ASSERT(f["x"].is(Var::NUL));

	ASL_ASSERT(Xdl::encode("a\nb") == "\"a\\nb\"");

	ASL_ASSERT(Json::encode( (Var(), 1, Var::NUL, false) ) == "[1,null,false]");

	ASL_ASSERT(!decodeXDL("1.25e08").is(Var::NONE));
	ASL_ASSERT(!decodeXDL("1.25e+08").is(Var::NONE));
	ASL_ASSERT(fabs( (double)decodeXDL("1.25e8") - 1.25e8) < 1e-6);
	ASL_ASSERT(fabs( (double)decodeXDL("1.25e+8") - 1.25e8) < 1e-6);

	ASL_ASSERT(Json::encode(nan()) == "null");
}

void testVar()
{
	Var b = Var("x", 3);
	ASL_ASSERT(b.type() == Var::DIC && b.length()==1 && b["x"]==3);

	Var c = array<Var>("x", 3, true, 0);
	ASL_ASSERT(c.type() == Var::ARRAY && c.length()==4 && c[0]=="x" && c[1]==3 && c[2]==true);
	ASL_ASSERT(c.contains("x"));
	ASL_ASSERT(c.contains(3));
	ASL_ASSERT(c.contains(true));
	ASL_ASSERT((bool)c && (bool)c[0] && (bool)c[1] && (bool)c[2] && !(bool)c[3])

	Var a = Var("x", 3)("y", 2);
	ASL_ASSERT(a.type() == Var::DIC);
	ASL_ASSERT(a.length()==2);
	ASL_ASSERT(a["x"]==3);
	ASL_ASSERT(a["y"]==2);
	ASL_ASSERT(a.has("x", Var::NUMBER));

	ASL_ASSERT((a("z") | a["x"]) == 3);

	Var a2 = a.clone();
	ASL_ASSERT(a2 == a);
	a2["z"] = c;
	ASL_ASSERT(a2 != a);
	ASL_ASSERT(a.has("y"));
	a.object().remove("y");
	ASL_ASSERT(!a.has("y"));
	c.array().remove(0);
	ASL_ASSERT(c.length() == 3);

	String s = "hello";
	a = s;
	String s2 = a;
	ASL_ASSERT(a.is(Var::STRING) && a == "hello");
	ASL_ASSERT(s2 == "hello");

	s = "My taylor is rich";
	a = s;
	s2 = a;
	ASL_ASSERT(a.is(Var::STRING) && a == "My taylor is rich");
	ASL_ASSERT(s2 == "My taylor is rich");

	a = "My taylor is not rich";
	s2 = a;
	ASL_ASSERT(a.is(Var::STRING) && a == "My taylor is not rich");
	ASL_ASSERT(s2 == "My taylor is not rich");

	a = 3;
	int i = a;
	ASL_ASSERT(a.is(Var::NUMBER) && a == 3);
	ASL_ASSERT(i == 3);

	a = false;
	bool f = a;
	ASL_ASSERT(a.is(Var::BOOL) && a == false);
	ASL_ASSERT(!f);

	String s3 = "a";
	Var v2 = s3 + "b";
	s3 = v2;
	ASL_ASSERT(s3 == "ab");

	ASL_ASSERT( Var(Var::NUL) == Var(Var::NUL) );

	Var none;
	ASL_ASSERT(!none);
	ASL_ASSERT(!(bool)none);

	int x = 9;
	a2.read("X", x);
	ASL_ASSERT(x == 9);
	a2.read("x", x);
	ASL_ASSERT(x == 3);
#ifdef ASL_HAVE_INITLIST
	Var a3 = Var::array({1, "a"});
	ASL_ASSERT(a3.is(Var::ARRAY) && a3.length() == 2 && a3[0] == 1 && a3[1] == "a");
#endif
}


void testBase64()
{
	String input = "2001-A Space Odyssey";
	String b64 = encodeBase64(input);
	ASL_ASSERT(b64 == "MjAwMS1BIFNwYWNlIE9keXNzZXk=");
	String c = decodeBase64(b64);
	ASL_ASSERT(c == input);
	Array<byte> data = array<byte>(0x05, 0xf0, 0x7a, 0x45);
	b64 = encodeBase64(data);
	ASL_ASSERT(b64 == "BfB6RQ==");
	ASL_ASSERT(decodeBase64(b64) == data);
	String h = encodeHex(data, data.length());
	ASL_ASSERT(h == "05f07a45");
	Array<byte> data2 = decodeHex(h);
	ASL_ASSERT(data == data2);
	String b64w = " MjAwMS\n1BIFN\n\twYWNlIE 9keXNzZXk = \n"; // with whitespace
	ASL_ASSERT(String(decodeBase64(b64w)) == input);
}


#define TEST( T ) else if(!strcmp(argv[1], #T)) test##T();

int main(int narg, char* argv[])
{
	{
		CmdArgs args(narg, argv);
		if (args.has("subproc")) {
			printf("subprocess %s\n", *args.all().slice(1).join(","));
			return args.all().length();
		}
	}
	if (narg < 2) {
		printf("No arguments\n");
		return EXIT_SUCCESS;
	}
	TEST(Var)
	TEST(XDL)
	TEST(XML)
	TEST(Array)
	TEST(CmdArgs)
	TEST(String)
	TEST(TabularDataFile)
	TEST(IniFile)
	TEST(Map)
	TEST(HashMap)
	TEST(Factory)
	TEST(StaticSpace)
	TEST(File)
	TEST(Path)
	TEST(Base64)
	TEST(AtomicCount)
	TEST(Process)
	TEST(SHA1)
	TEST(SmartObject)
	TEST(Date)
	TEST(Vec3)
	TEST(Uuid)
	else
		return EXIT_FAILURE;
	
	return EXIT_SUCCESS;
}
