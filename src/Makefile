WEBAPS:../cgi-bin/airLock ../cgi-bin/dashboard ../cgi-bin/databaseSync ../cgi-bin/dataGather \
 ../cgi-bin/everyone ../cgi-bin/ingestReport ../cgi-bin/home ../cgi-bin/libraryView \
 ../cgi-bin/modbusTcpIp ../cgi-bin/moveReportToDb \
 ../cgi-bin/panoViewer ../cgi-bin/photoAlbum ../cgi-bin/photoUpload ../cgi-bin/pointEnter \
 ../cgi-bin/quickGuideWrapper ../cgi-bin/reportGenWrapper ../cgi-bin/reportMailSender \
 ../cgi-bin/reportReview ../cgi-bin/roster ../cgi-bin/routePlanner \
 ../cgi-bin/temperatureGraph ../cgi-bin/tileServer

# skipping ../cgi-bin/imageDownForUp which will prepare lowres picture proofs for reports
# it depend on cimg which may not currently be installed at MDRS

../cgi-bin/airLock:airLock.c
	/bin/sh ./sssscccc $<
	g++ -O -o $@ $<

../cgi-bin/dashboard:dashboard.c session.o dataBrane.o timeString.o
	/bin/sh ./sssscccc $<
	g++ -O -o $@ $< dataBrane.o timeString.o -lpqxx

../cgi-bin/databaseSync:databaseSync.c session.o dataBrane.o timeString.o
	/bin/sh ./sssscccc $<
	g++ -O -o $@ $< session.o dataBrane.o timeString.o -lpqxx

../cgi-bin/dataGather:dataGather.c session.o dataBrane.o timeString.o
	/bin/sh ./sssscccc $<
	g++ -O -o $@ $< session.o dataBrane.o timeString.o -lpqxx

../cgi-bin/everyone:everyone.c session.o dataBrane.o timeString.o
	./sssscccc $<
	g++ -O -o $@ $< session.o dataBrane.o timeString.o -lpqxx

../cgi-bin/home:home.c session.o dataBrane.o timeString.o
	./sssscccc $<
	g++ -O -o $@ $< session.o dataBrane.o timeString.o -lpqxx

../cgi-bin/ingestReport:ingestReport.c dataBrane.o timeString.o
	./sssscccc $<
	g++ -O -o $@ $< dataBrane.o timeString.o -lpqxx

../cgi-bin/imageDownForUp:imageDownForUp.cpp
	g++ -O -o $@ $< -L/usr/X11R6/lib -lm -lpthread -lX11

../cgi-bin/libraryView:libraryView.c timeString.o session.o dataBrane.o svgGraph.o
	./sssscccc $<
	g++ -O -o $@ $< timeString.o session.o dataBrane.o svgGraph.o -lpqxx

../cgi-bin/modbusTcpIp:modbusTcpIp.c dataBrane.o  timeString.o
	./sssscccc $<
	g++ -O -o $@ $< dataBrane.o timeString.o -lpqxx

../cgi-bin/moveReportToDb:moveReportToDb.c dataBrane.o  timeString.o
	./sssscccc $<
	g++ -O -o $@ $< dataBrane.o timeString.o -lpqxx

../cgi-bin/panoViewer:panoViewer.c session.o dataBrane.o timeString.o
	./sssscccc $<
	g++ -O -o $@ $< session.o dataBrane.o timeString.o -lpqxx

../cgi-bin/photoAlbum:photoAlbum.c session.o dataBrane.o timeString.o
	./sssscccc $<
	g++ -O -o $@ $< session.o dataBrane.o timeString.o -lpqxx -lcgicc

../cgi-bin/photoUpload:photoUpload.c session.o dataBrane.o timeString.o
	./sssscccc $<
	g++ -O -o $@ $< session.o dataBrane.o timeString.o -lpqxx -lcgicc

../cgi-bin/pointEnter:pointEnter.c session.o dataBrane.o timeString.o
	./sssscccc $<
	g++ -O -o $@ $< session.o dataBrane.o timeString.o -lpqxx

../cgi-bin/quickGuideWrapper:quickGuideWrapper.c session.o dataBrane.o timeString.o
	./sssscccc $<
	g++ -O -o $@ $< session.o dataBrane.o timeString.o -lpqxx

../cgi-bin/reportGenWrapper:reportGenWrapper.c session.o dataSummary.o dataBrane.o timeString.o
	./sssscccc $<
	g++ -O -o $@ $< session.o dataSummary.o dataBrane.o timeString.o -lpqxx

../cgi-bin/reportMailSender:reportMailSender.c session.o dataSummary.o dataBrane.o timeString.o
	./sssscccc $<
	g++ -O -o $@ $< session.o dataSummary.o dataBrane.o timeString.o -lpqxx

../cgi-bin/reportReview:reportReview.c session.o dataSummary.o dataBrane.o timeString.o
	./sssscccc $<
	g++ -O -o $@ $< session.o dataSummary.o dataBrane.o timeString.o -lpqxx

../cgi-bin/roster:roster.c session.o dataBrane.o timeString.o
	./sssscccc $<
	g++ -O -o $@ $< session.o dataBrane.o timeString.o -lpqxx

../cgi-bin/routePlanner:routePlanner.c session.o dataBrane.o timeString.o
	./sssscccc $<
	g++ -O -o $@ $< session.o dataBrane.o timeString.o -lpqxx

../cgi-bin/temperatureGraph:temperatureGraph.c session.o dataBrane.o timeString.o
	./sssscccc $<
	g++ -O -o $@ $< session.o dataBrane.o timeString.o -lpqxx

../cgi-bin/tileServer:tileServer.c session.o dataBrane.o timeString.o
	./sssscccc $<
	g++ -O -o $@ $< session.o dataBrane.o timeString.o -lpqxx


# ../cgi-bin/snagWaypoint:snagWaypoint.c
# 	g++ -O -o $@ $<

### objects
dataBrane.o:dataBrane.c
	g++ -O -c dataBrane.c
	./sssscccc $<

dataSummary.o:dataSummary.c
	g++ -O -c dataSummary.c
	./sssscccc $<

session.o:session.c
	g++ -O -c session.c
	./sssscccc $<

svgGraph.o:svgGraph.c
	g++ -O -c $<
	./sssscccc $<

timeString.o:timeString.c
	g++ -O -c $<
	./sssscccc $<

### tests
db_test:db_test.c dataBrane.o
	g++ -O -o db_test db_test.c dataBrane.o -lpqxx
	./sssscccc $<
