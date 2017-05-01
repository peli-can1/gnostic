/******************************************************************************/
/**
 * \file    Trace.hpp
 *
 * \author  Peter Lidbjork
 *
 * Copyright &copy; Maquet Critical Care AB, Sweden
 *
 ******************************************************************************/

/*
 *
 * Trace is a macro based tool inspired by the DBUG library by Fred Fish. It is intended for tracing and debugging of code.
 * To use Trace, USE_TRACE must be defined when building.
 *
 * The behavior of Trace is controlled by an options string, supplied by the 'setOptions' method.
 * This makes it possible to control Trace with command line arguments, i.e. without the need to rebuild.
 * One letter in the string corresponds to one feature.To enable some features for instance call Trace::setOptions("fimnpt");
 * Meaning of letters:
 * 'f' print file name
 * 'l' print line number
 * 'm' print the time in milliseconds a method took to execute.
 * 'i' print thread id
 * 'n' print thread name, that was earlier provided by setThreadName call.
 * 'p' print strings provided in TRACE_PRINT macro.
 * 't' print traversed function names with nesting level.
 * 'd' print date and time for each string.
 * 'c' print out strings generated by TRACE_CHECK. Otherwise just execute the call silently.
 * 'r' print row numbers.
 *
 * The options must be specified per thread, e.g. in the QThread::run method. This makes it possible to debug threads separately.
 * QThread::currentThreadId() is used internally to bind options to a specific thread.
 *
 * Macros:
 * TRACE_ENTER("function name"). Must be called in a function to enable tracing.
 * TRACE_RETURN(value). Used instead of 'return value' to obtain line number of the return when tracing.
 * TRACE_VOID_RETURN. Used instead of 'return' to obtain line number of the return when tracing.
 * TRACE_PRINT: Used to print arbitrary strings. Has printf style argument list. Can also take a keyword to filter output.
 *    Example: TRACE_PRINT("mytest",("Value returned %d", aValue));
 *
 * Filtering output: To print only lines with a special keyword, use the method Trace::setRegExpStr(). Then only lines tagged with
 * a keyword that satisfies the regular expression will be printed by the TRACE_PRINT macro.
 **/


#ifndef TRACE_HPP
#define TRACE_HPP

#define USE_TRACE 1
#ifdef USE_TRACE

#define TR_TAB "    "
#define TR_TAB2 "        "

#include <string>
#include <sstream>
#include <vector>
#include <mutex>
#include <thread>
#include <map>
#include <fstream>
#include <atomic>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/timer.hpp>

#ifdef TRACE
#undef TRACE
#endif
    #define TRACE_READ_CONFIG_FILE(app,path) Trace::readConfig(app,path);
    #define TRACE_CREATE_CONTEXT(a,b) Trace::createContext(a,b);
    #define TRACE_SET_LOG_STREAM(a) Trace::setLogStream(a);
    #define TRACE() Trace __traceObject__(__func__ , __FILE__, __LINE__)
    #define TRACE_ENTER(a) Trace __traceObject__(a , __FILE__, __LINE__)
    #define TRACE_RETURN(a) __traceObject__.out(__LINE__);return a;
    #define TRACE_VOID_RETURN __traceObject__.out(__LINE__);return;
    #define TRACE_PRINT(keyword, argList) {__traceObject__.printState(keyword, __FILE__, __LINE__, Trace::printArgs argList);}
    #define TRACE_PROF_START {__traceObject__.profTimerStart(__LINE__);}
    #define TRACE_PROF_ELAPSED {__traceObject__.profTimerElapsed(__LINE__);}
    #define TRACE_CHECK(a) __traceObject__.check(#a, a, __LINE__);
    #define TRACE_DISABLE __traceObject__.disable();
    #define TRACE_ENABLE __traceObject__.enable();
    #define TRACE_CLOSE_LOGFILE Trace::closeLogFile();
    #define TRACE_SET_TIME_ELAPSED_START Trace::setTimeElapsedStart();
    #define TRACE_COMPARE(a,b) __traceObject__.compare(#a,#b, a, b, __LINE__)
    #define TRACE_FLUSH __traceObject__.flush();

    class Trace
    {
    public:
        // Below is only internal stuff, do not use explicitly!
        typedef  unsigned long options_t;

        struct Configuration  {
            explicit Configuration(){options=0;}
            std::string name;
            options_t options;
            std::string prompt;
			std::string simpleSearchStr;
            std::string regexpStr;
            std::string logFileName_;
            std::string logFileMode_;

            friend std::ostream& operator<<(std::ostream& os, const Configuration& c); 
        };        
        struct Context {
            explicit Context(){nestingLevel=0;conf=nullptr;logStream_=nullptr;}
            std::thread::id threadId;
            int nestingLevel;
            Configuration* conf;
            std::ostream* logStream_;
            std::ofstream logFile_;

            friend std::ostream& operator<<(std::ostream& os, const Context& c); 
        };

		static bool readConfig(const std::string& appName, const std::string& pathToConfigFile);
        static void createContext(const std::string& name, const std::string& opts);
//		static void disable(const std::string& file, const int line);
        static void disable(){s_disabled = true;}
        static void enable(){s_disabled = false;}

        static void closeLogFile();

        
        // static int getopt(int nargc, char * const nargv[], const char *ostr);    
		explicit Trace(const std::string& func, const std::string& file, const int line);
        void out(const int line);
		void flush();
		void printState(const std::string& keyword, const char* file, int line, char* args);
        static char* printArgs(const char* format, ...);
        ~Trace();
        void profTimerStart(int lineNo);
        void profTimerElapsed(int lineNo);
        void check(const char* expression, bool result, int line);

        void compare(const char* first, const char* second, int firstVal, int secondVal, int lineNo);
        void compare(const char* first, const char* second, unsigned int firstVal, unsigned int secondVal, int lineNo);
        void compare(const char* first, const char* second, float firstVal, float secondVal, int lineNo);
        void compare(const char* first, const char* second, double firstVal, double secondVal, int lineNo);
        void compare(const char* first, const char* second, char firstVal, char secondVal, int lineNo);
        void compare(const char* first, const char* second, unsigned char firstVal, unsigned char secondVal, int lineNo);

        static void setTimeElapsedStart();
        static options_t parseOptions(const std::string&);
        
        // Set context attributes from code. Call from appropriate thread!
        static void setName(const std::string& name);
        static void setOptions(options_t options);
        std::string simpleSearchStr;
            std::string regexpStr;
            std::string prompt;

        // Getopt variables
        /*
        static int opterr;
        static int optopt;
        static int optind;
        static char * optarg;
*/
    private:
        static void setSimpleSearchStr(const std::string&);
        static void setRegExpStr(const std::string&); // Sets regular expression for the current thread.
        static void setLogFile(FILE*);   // Sets global output file.
		static void setLogFile(const std::string& fileName, bool overWrite=true); // Opens and sets global output file.
		static void setPrompt(const std::string&); // Sets the first word on each line.
		void compareHelper(const char* first, const char* second, int result, int lineNo, const std::string& valStr1="", const std::string& valStr2="");

        static Context* context();
		static void traceOut(const Context* ct, const std::string& extra, const std::string& funcName, const std::string& args, const std::string& fileName, int lineNo, double  ms = -1.0); // Construct string based on options.
        static void setLogStream(Context&);

		static std::vector<Context*> contexts_; // One context per thread
        // static QMutex mutex_;
        static std::mutex mutex_;

		std::string  funcName_;
        std::string fileName_;
        int line_;
        int exitLine_;
        // QTime time_;
        boost::timer timer_;


        // Attributes for "profiling".
        int profStartLine_;
        int profEndLine_;
        // QTime profTime_;
        boost::timer profTime_;

        static FILE* logFile_;
        static std::ostream* m_logStream;
        //static std::ofstream logFile_; 
        
        //static QMap<QString, Configuration*> configMap_;
        static std::map<std::string, Configuration*> configMap_;

        static std::atomic<bool> s_disabled;

        static boost::timer timeElapsedStart_;
        static options_t s_globalOptions; // Shared by all contexts.
        // UDP stuff
/*
		static int udpPort_;
        static bool udpActive_;
        static QHostAddress hostAddress_; // Address of remote host, or Any
        
        static QUdpSocket socket_;
		static bool activateUdp();
*/
    };
#else // USE_TRACE

    #define TRACE_ENTER(a)
    #define TRACE()
    #define TRACE_RETURN(a) return a;
    #define TRACE_VOID_RETURN return;
    #define TRACE_PRINT(keyword, argList)
    #define TRACEF_PRINT(argList)
    #define TRACE_PROF_START
    #define TRACE_PROF_ELAPSED
    #define TRACE_CHECK(a) a
    #define TRACE_CREATE_CONTEXT(a)
    #define TRACE_READ_CONFIG_FILE
    #define TRACE_DISABLE
    #define TRACE_ENABLE
    #define TRACE_CLOSE_LOGFILE
    #define TRACE_SET_TIME_ELAPSED_START
    #define TRACE_COMPARE(a,b)
    #define TRACE_FLUSH
    #endif // USE_TRACE

#endif // TRACE_HPP