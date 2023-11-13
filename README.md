# VsdNano
This project makes visualization of ntuples with the ROOT REve module. It contains a Makefile that builds reveNtuple binary that takes an argument for data source and json to configure mapping between Tree auto generated class and REve VSD structures defined in [VsdBase.h](VsdBase.h). After bould the one can run the event display with command.
```
./reveNutple <data.root> <config.sjon>
```


### To run
Setup ROOT enviroment with root master or rootversion 6.28 or newer.
Bould sources running the `make` command

### Examples
Run makefile target `test` to download en example sources with and example json configuration

```
make test
```

### Troubleshooting
When example is run remotely one may need run event display server at fixed port and create a ssh tunnel.

```
# cat .rootrc
WebGui.HttpPort 1234
```
Make an ssh tunner from server running on port e.g. 1234 to localhost to access evnet display with URL localhost:7777/win1/

```
ssh -f -L 1234:localhost:7777 cms-ed-1 sleep 10000
```
