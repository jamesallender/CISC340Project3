CC= gcc
CFLAGS=`pkg-config --cflags gmodule-2.0`

LDFLAGS=`pkg-config --libs gmodule-2.0`

all: pipelineSimulatorJames.o
	$(CC) $(CFLAGS) pipelineSimulatorJames.o -o pipelineSimulatorJames $(LDFLAGS)

pipelineSimulatorJames.o: pipelineSimulatorJames.c
	$(CC) $(CFLAGS) -c pipelineSimulatorJames.c

clean:
	rm pipelineSimulatorJames *.o *.*~ *~
