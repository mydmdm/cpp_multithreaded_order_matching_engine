Licence : All samples are "Public Domain" code 
http://en.wikipedia.org/wiki/Public_domain_software

Sections :

	1. Introduction
	2. Limit orders and order matching engines
	3. FIX ( Financial Information Exchange ) protocol
	4. Build dependencies
	5. Runtime dependencies
	6. How to build
	7. Server parameters and running the server
	8. Example log messages with FIX 
	9. Functional testing
	10. Unit testing with GoogleTest
	11. Coding and other guidelines
	12. TODO List

===========================================================================
			
**1. Introduction :** A multithreaded order matching engine written in C++11 using FIX protocol 4.2 . 

- It targets both Linux ( tested on CentOS and Ubuntu ) and Windows systems ( tested on Windows 8.1).

- It uses STL,Boost and also platform specific APIs ( POSIX, a few NP POSIX calls and WindowsAPIs) in some places, QuickFix for FIX protocol.

- It supports GCC4.8 and MSVC120 (VS2013). See "source/compiler_portability" 

- Additionally used GoogleTest for unit testing, Bash for Linux test scripts and Powershell for Windows test scripts.

- Currently supports only limit orders and currently doesn`t support TIF.

- See end of this readme for future plans.

Regarding the implementation details and performance gain in multithreading and other considerations , please see the article on :

https://nativecoding.wordpress.com/2016/02/07/multithreading-considerations-in-an-order-matching-engine/

===========================================================================
						
**2. Limit orders and order matching engines :** For limit orders please see : https://en.wikipedia.org/wiki/Order_%28exchange%29#Limit_order

Basically a limit order is an order which you specify a maximum price for the security you want to buy. 
As for the terminology a limit order to buy is called a "bid" and a limit order to sell is called an "ask".

An order matching engine matches the ask orders and the bid orders. This implementation places the highest bid order on
top of bids table and places the lowest ask order on top of asks table. Then it chekcs the bids and asks and tries to 
match the orders. And then it sends status reports back to the owners of the orders. A status of an order can be :

- "accepted" : the server accepted the order and it will be processed
- "filled" : the order matched
- "partially filled" , meaning that some trading happened , but still more to process
- "rejected" , if order type is not supported.
- "canceled" , if a client wants to cancel an order ,and if the order is canceled , the server informs the client

For general information about the trading systems and the order types , please see :
http://www.investopedia.com/university/intro-to-order-types/

===========================================================================

**3. FIX ( Financial information exchange ) protocol :** It is a session based TCP protocol that carries financial security transcation data.

For more information , please see https://en.wikipedia.org/wiki/Financial_Information_eXchange .

For the time being, this projectis using opensource QuickFix engine and FIX specification 4.2.
	
===========================================================================

**4. Build dependencies :** For Linux , the project is built and tested with GCC4.8 only on CentOS7. 

As for Windows it is using MSVC1200(VS2013). An important note about VS2013 , its version shouldn`t be later then Update2 as the project is using C++11 curly brace initialisation in MILs and MSVC rollbacked that feature starting from Update3 :

https://connect.microsoft.com/VisualStudio/feedbackdetail/view/938122/list-initialization-inside-member-initializer-list-or-non-static-data-member-initializer-is-not-implemented

In the libraries side :

- Boost 1.59 : Using only template based part of Boost and a compacted version of Boost is in dependencies directory, therefore you don`t need to do anything. The compacting command is as below  :

				bcp --boost=c:\boost_1_59_0 shared_ptr scoped_ptr any optional tokenizer format c:\boost
						
- QuickFix & its requirements : For Windows you don`t need to do anything as the static library for Windows is already in dependencies directory. For Linux you need to apply the steps on http://www.quickfixengine.org/quickfix/doc/html/install.html

===========================================================================

**5. Runtime dependencies :** For Windows, you have to install MSVC120 runtime : https://www.microsoft.com/en-gb/download/details.aspx?id=40784

For Linux, you need GNU Libstd C++ 6 runtime and QuickFIX runtime.

How to install Quickfix runtime on Linux ( tested on Ubuntu ) :

		1. Navigate to dependencies/quickfix/linux_runtime
		2. sudo chmod +x ./install_quickfix_runtime.sh
		3. sudo ./install_quickfix_runtime.sh
		
Note : This script will copy shared object to library path, create soft links, will add library path to /etc/ld.so.conf and finally execute ldconfig.
		
===========================================================================

**6. How to build :**
			
How to build the project on Linux :
	
	cd build/linux
	make clean
	make debug  OR make release

How to build the project on Linux using Netbeans 8.0.2 C++ IDE:

	Open Netbeans.
	Open the project from the project directory. ( Choose "nbproject" directory )
	Build the project inside Netbeans IDE.

Why Netbeans : In Netbeans, it is too straightforward to setup remote debugging, therefore it is quite convenient to build and debug on Linux from Windows via SSH and Samba. You can see an article about this setup here in my blog. It is for Debian but it should be trivial to apply it to any other distribution : https://nativecoding.wordpress.com/2014/10/24/configuring-a-debian-virtual-machine-for-linux-c-development-via-windows-step-by-step/
	
How to build the project on Windows  :
	
	You can build with Visual Studio 2013
	Go to "build/windows" directory
	Use SLN file to launch VS with the project
	
===========================================================================

**7. Server parameters and running the matching engine :** The engine executable looks for "ome.ini" file. Here is the list of things you can set :

		FILE_LOGGING_ENABLED						enables/disables logging
		CONSOLE_OUTPUT_ENABLED						enables/disables output to stdout
		CENTRAL_ORDER_BOOK_PIN_THREADS_TO_CORES		whether to pin threads of the threadpool to different CPU cores
		HYPER_THREADING								if hyperthreading is off and pinning is on ,then it will pin threads to only cores with an even index
		CENTRAL_ORDER_BOOK_QUEUE_SIZE_PER_THREAD	Queue size per worker thread in the central order book`s thread pool
		LOG_BUFFER_SIZE								Maximum buffer size for the logging system as it is built on a ring buffer.
		
You will also need to specify security symbols. The order matching engine`s thread pool will create a worker thread for each symbol.
For specifying symbols in ini file, you need to use brackets as  below :

		SYMBOL[]=MSFT
		SYMBOL[]=AAPL
		SYMBOL[]=INTC
		SYMBOL[]=GOOGL
		
You will also need to have "quickfix_FIX42.xml" and "quickfix_server.cfg" files to be in the same directory with OME executable. You can find them in "bin" directory.

Once you start the ome executable , initially you will see a screen like this :

		06-02-2016 16:22:00 : INFO , Main thread , starting
		06-02-2016 16:22:00 : INFO , Thread pool , Thread(0) MSFT starting
		06-02-2016 16:22:00 : INFO , Thread pool , Thread(1) AAPL starting
		06-02-2016 16:22:00 : INFO , Thread pool , Thread(2) INTC starting
		06-02-2016 16:22:00 : INFO , Thread pool , Thread(3) GOOGL starting
		06-02-2016 16:22:00 : INFO , Thread pool , Thread(4) QCOM starting
		06-02-2016 16:22:00 : INFO , Thread pool , Thread(5) QQQ starting
		06-02-2016 16:22:00 : INFO , Thread pool , Thread(6) BBRY starting
		06-02-2016 16:22:00 : INFO , Thread pool , Thread(7) SIRI starting
		06-02-2016 16:22:00 : INFO , Thread pool , Thread(8) ZNGA starting
		06-02-2016 16:22:00 : INFO , Thread pool , Thread(9) ARCP starting
		06-02-2016 16:22:00 : INFO , Thread pool , Thread(10) XIV starting
		06-02-2016 16:22:00 : INFO , Thread pool , Thread(11) FOXA starting
		06-02-2016 16:22:00 : INFO , Thread pool , Thread(12) TVIX starting
		06-02-2016 16:22:00 : INFO , Thread pool , Thread(13) YHOO starting
		06-02-2016 16:22:00 : INFO , Thread pool , Thread(14) HBAN starting
		06-02-2016 16:22:00 : INFO , Thread pool , Thread(15) BARC starting
		06-02-2016 16:22:00 : INFO , Incoming message dispatcher , Thread starting
		06-02-2016 16:22:00 : INFO , Outgoing message processor , Thread starting
		06-02-2016 16:22:00 : INFO , FIX Engine , Acceptor started

		Available commands :

				display : Shows all order books in the central order book
				quit : Shutdowns the server
				
===========================================================================
				
**8. Example log message from the engine :** The engine produces log messages below when it receives 1 buy order with quantity 1 and 1 sell order with quantity 1 for the same symbol :

	06-02-2016 20:16:09 : INFO , FIX Engine , New logon , session ID : FIX.4.2:OME->TEST_CLIENT1
	06-02-2016 20:16:09 : INFO , FIX Engine , Sending fix message : 8=FIX.4.29=15435=834=543=Y49=OME52=20160206-20:16:09.29556=TEST_CLIENT1122=20160206-20:15:03.9556=011=414=017=1820=037=438=139=054=155=MSFT150=0151=110=000
	06-02-2016 20:16:09 : INFO , FIX Engine , Sending fix message : 8=FIX.4.29=15535=834=643=Y49=OME52=20160206-20:16:09.29556=TEST_CLIENT1122=20160206-20:15:03.9556=011=514=017=1920=037=538=239=054=255=GOOGL150=0151=210=070
	06-02-2016 20:16:09 : INFO , FIX Engine , Sending fix message : 8=FIX.4.29=15535=834=743=Y49=OME52=20160206-20:16:09.29556=TEST_CLIENT1122=20160206-20:15:03.9716=011=614=017=2020=037=638=139=054=155=GOOGL150=0151=110=060
	06-02-2016 20:16:09 : INFO , FIX Engine , Sending fix message : 8=FIX.4.29=16435=834=843=Y49=OME52=20160206-20:16:09.29556=TEST_CLIENT1122=20160206-20:15:03.9716=111=414=117=2120=031=132=137=438=139=254=155=MSFT150=2151=010=168
	06-02-2016 20:16:09 : INFO , FIX Engine , Sending fix message : 8=FIX.4.29=16435=834=943=Y49=OME52=20160206-20:16:09.29556=TEST_CLIENT1122=20160206-20:15:03.9716=111=314=117=2220=031=132=137=338=139=254=255=MSFT150=2151=010=169
	06-02-2016 20:16:09 : INFO , FIX Engine , Sending fix message : 8=FIX.4.29=15635=834=1043=Y49=OME52=20160206-20:16:09.29556=TEST_CLIENT1122=20160206-20:15:03.9716=011=714=017=2320=037=738=139=054=155=GOOGL150=0151=110=108
	06-02-2016 20:16:09 : INFO , FIX Engine , Sending fix message : 8=FIX.4.29=15435=834=1143=Y49=OME52=20160206-20:16:09.29556=TEST_CLIENT1122=20160206-20:15:03.9716=011=814=017=2420=037=838=139=854=155=xxx150=8151=110=110
	06-02-2016 20:16:09 : INFO , FIX Engine , Sending fix message : 8=FIX.4.29=16635=834=1243=Y49=OME52=20160206-20:16:09.29556=TEST_CLIENT1122=20160206-20:15:03.9716=111=614=117=2520=031=132=137=638=139=254=155=GOOGL150=2151=010=027
	06-02-2016 20:16:09 : INFO , FIX Engine , Sending fix message : 8=FIX.4.29=16635=834=1343=Y49=OME52=20160206-20:16:09.29556=TEST_CLIENT1122=20160206-20:15:03.9716=111=514=117=2620=031=132=137=538=239=154=255=GOOGL150=1151=110=028
	06-02-2016 20:16:09 : INFO , FIX Engine , Sending fix message : 8=FIX.4.29=16635=834=1443=Y49=OME52=20160206-20:16:09.29556=TEST_CLIENT1122=20160206-20:15:03.9716=111=714=117=2720=031=132=137=738=139=254=155=GOOGL150=2151=010=033
	06-02-2016 20:16:09 : INFO , FIX Engine , Sending fix message : 8=FIX.4.29=16635=834=1543=Y49=OME52=20160206-20:16:09.29556=TEST_CLIENT1122=20160206-20:15:03.9716=111=514=217=2820=031=132=137=538=239=254=255=GOOGL150=2151=010=034
	06-02-2016 20:16:09 : INFO , FIX Engine , Receiving fix message : 8=FIX.4.29=12335=D34=1549=TEST_CLIENT152=20160206-20:16:09.34256=OME11=121=138=140=244=154=255=MSFT59=060=20160206-20:16:0910=124
	06-02-2016 20:16:09 : INFO , FIX Engine , New order message received :8=FIX.4.29=12335=D34=1549=TEST_CLIENT152=20160206-20:16:09.34256=OME11=121=138=140=244=154=255=MSFT59=060=20160206-20:16:0910=124
	06-02-2016 20:16:09 : INFO , Central Order Book , New order accepted, client TEST_CLIENT1, client order ID 1 
	06-02-2016 20:16:09 : INFO , Outgoing message processor , Processing ACCEPTED for order : Client TEST_CLIENT1 Client ID 1 Symbol MSFT Side SELL 
	06-02-2016 20:16:09 : INFO , Thread pool , MSFT thread got a new task to execute
	06-02-2016 20:16:09 : INFO , FIX Engine , Sending fix message : 8=FIX.4.29=12535=834=2349=OME52=20160206-20:16:09.34256=TEST_CLIENT16=011=114=017=11320=037=138=139=054=255=MSFT150=0151=110=079
	06-02-2016 20:16:09 : INFO , Central Order Book , Order processing for symbol MSFT took 0000000 milliseconds , num of processed orders : 0
	06-02-2016 20:16:09 : INFO , FIX Engine , Receiving fix message : 8=FIX.4.29=12335=D34=1649=TEST_CLIENT152=20160206-20:16:09.34256=OME11=221=138=140=244=154=155=MSFT59=060=20160206-20:16:0910=125
	06-02-2016 20:16:09 : INFO , FIX Engine , New order message received :8=FIX.4.29=12335=D34=1649=TEST_CLIENT152=20160206-20:16:09.34256=OME11=221=138=140=244=154=155=MSFT59=060=20160206-20:16:0910=125
	06-02-2016 20:16:09 : INFO , Central Order Book , New order accepted, client TEST_CLIENT1, client order ID 2 
	06-02-2016 20:16:09 : INFO , Outgoing message processor , Processing ACCEPTED for order : Client TEST_CLIENT1 Client ID 2 Symbol MSFT Side BUY 
	06-02-2016 20:16:09 : INFO , Thread pool , MSFT thread got a new task to execute
	06-02-2016 20:16:09 : INFO , FIX Engine , Sending fix message : 8=FIX.4.29=12535=834=2449=OME52=20160206-20:16:09.34256=TEST_CLIENT16=011=214=017=11420=037=238=139=054=155=MSFT150=0151=110=082
	06-02-2016 20:16:09 : INFO , Central Order Book , Order processing for symbol MSFT took 0000000 milliseconds , num of processed orders : 2
	06-02-2016 20:16:09 : INFO , Outgoing message processor , Processing FILLED for order : Client TEST_CLIENT1 Client ID 2 Symbol MSFT Side BUY 
	06-02-2016 20:16:09 : INFO , FIX Engine , Sending fix message : 8=FIX.4.29=13535=834=2549=OME52=20160206-20:16:09.34256=TEST_CLIENT16=111=214=117=11520=031=132=137=238=139=254=155=MSFT150=2151=010=001
	06-02-2016 20:16:09 : INFO , Outgoing message processor , Processing FILLED for order : Client TEST_CLIENT1 Client ID 1 Symbol MSFT Side SELL 
	06-02-2016 20:16:09 : INFO , FIX Engine , Sending fix message : 8=FIX.4.29=13535=834=2649=OME52=20160206-20:16:09.34256=TEST_CLIENT16=111=114=117=11620=031=132=137=138=139=254=255=MSFT150=2151=010=002
	06-02-2016 20:16:11 : INFO , FIX Engine , Logout , session ID : FIX.4.2:OME->TEST_CLIENT1

Note 1: To parse the fix message which can be seen above , you can use one of the online FIX parsers :

http://fixdecoder.com/fix_decoder.html
		
http://fixparser.targetcompid.com/
	
http://fix.aprics.net/

Note 2: The executables and scripts help you to send orders to the engine , however if you want to send custom FIX messages, you can use :

MiniFIX , http://elato.se/minifix/download.html , Windows only with a GUI

QuickFixMessanger , https://github.com/jramoyo/quickfix-messenger

===========================================================================

**9. Functional testing :** There is a prebuilt executable for both Linux and Windows which can send specified ask/bid orders to the order matching engine.
   
   Under "test_functional" directory :
   
		1. Modify test_data.txt which has the orders to send to the engine as you wish.
		
		2. Modify  the arrays declared on top of client_automated_test.sh/client_automated_test.ps1 script files in order to configure the number of clients. You should provide a name for each client.
		
		3. For firing Linux test client(s), you can use client_automated_test.sh Bash script.
		
		4. For firing Windows test client(s), you can use client_automated_test.bat which drives client_automated_test.ps1 Powershell script.
		
		5. After firing the script, it will be executing all orders in test_data.txt file per client that is declared in the script file.
		
===========================================================================
		
**10. Unit testing with GoogleTest :** The project uses GoogleTest 1.7. You can find a makefile and vcproj under "test_unit" directory.

	
Building and running unit test on Linux : You have to build and install Google Test 1.7 first , the instructions for CentOS and Ubuntu :
			
			$ wget http://googletest.googlecode.com/files/gtest-1.7.0.zip
			$ unzip gtest-1.7.0.zip
			$ cd gtest-1.7.0
			$ ./configure
			$ make
			$ sudo cp -a include/gtest /usr/include
			
			On CentOS
			$ sudo cp -a lib/.libs/* /usr/lib64
			
			On Ubuntu
			$ sudo cp -a lib/.libs/* /usr/lib/
			
			Then you can either use Makefile or Netbeans project files under "test_unit" directory.

Building and running unit test on Windows : You can use VisualStudio solution in "test_unit" directory.

===========================================================================

**11. Coding and other guidelines :**

Source code and file/directory naming conventions :
	
	Directory names					lower_case_word
	File names 						lower_case_word
	Include guards 					_HEADER_H_
	Macros							UPPER_CASE_WORD
	Enums							UPPER_CASE_WORD
	Namespace names 				lower_case_word
	Class names 					CamelCase
	Method names 					pascalCase
	Variable names					pascalCase
	Member variables starts with	m_
	Directories/Namespaces			As in Boost, there is a namespace per directory

Source code indentations and new line usage :
	
	4 spaces, no tabs ( This needs to be setup in VS )
	By default Netbeans editor uses spaces for tabs
	Needs to be set in VS2013 : https://msdn.microsoft.com/en-gb/library/ms165330(v=vs.90).aspx
	New lines : Unix CR only ( \n ) , VisualStudio can handle it even though Windows is \r\n
	
Inclusions : Using forward slash as it works for both Linux and Windows :

		#include <concurrent/thread.h>
	
Warning level used for GCC : -Wall

Warning level used for MSVC : /W3
		
Precompiled header file usage : On Windows , the project is using /FI ( Force include parameter, therefore no need to include the pch header everywhere ) and specified the pch header to be precompiled_header.h. Note that this breaks edit-and-continue in Visual Studio.
For Linux , there is pch rule to enable it in the makefile ( build/linux/Makefile) , but currently that rule is not being used since it doesn`t appear as it is doing much improvement as on Windows.

For GCC see https://gcc.gnu.org/onlinedocs/gcc/Precompiled-Headers.html

For MSVC 120 see https://msdn.microsoft.com/en-us/library/8c5ztk84(v=vs.120).aspx

MSVC120 C++11 Limitations : Curly brace initialisation at MILs and noexcept is not supported. For noexcept usage please see compiler_portability/noexcept.h .

===========================================================================

**12. Todo List :**

Benchmarking & Microbenchmarking : Will add probes for SystemTap for Linux, might add performance test cases using existing GoogleTest project

Concurrency : MPMC and MPSC lockfree container implementations , currently only SPSC bounded queue is lock free.

Memory : 3rd party memory allocators support : jemalloc, intelTBB, tcMalloc, Lockless. Currently the engine is using a set of CPU cache aligned allocators in "source/memory".


**Other considerations for future :**

Order matching : Adding other order types ( market orders, stop loss order) , order update and market data request support, TIF support

Exchange connectivity : Support ITCH ( London Stock Exchange & NASDAQ ) and/or FIX engines , interested in using Libtrade : https://github.com/libtrading/libtrading

Compiler support : Use of CMake, upgrading supported compiler versions and enabling use of C++14/C++17, experimental Clang and IntelC++Compiler

Concurrency : Experiment order book processing on GPU with CUDA

OS Support : Solaris11

New feature : Event broadcasting and sample feed handlers

New feature : Visualisation of transactions with OpenGL/Vulkan. See links below : 

http://obt.hottolink.com/

http://parasec.net/transmission/order-book-visualisation/

Valve is sponsoring LunarG which is working on Vulkan drivers and SDK which is likely to be available this year :

http://lunarg.com/vulkan/

New feature : Save events in a database

===========================================================================