#cleos = cleos --url http://127.0.0.1:18888 --wallet-url http://127.0.0.1:8888 
cleos=cleos

all: build set

build:
	eosio-cpp -o kqjs.wasm kqjs.cpp --abigen

set:
	$(cleos) set contract kqjs /opt/eosio/bin/data-dir/contracts/kqjs -p kqjs@active 

createaccount:
	$(cleos) create account eosio kqjs EOS7HxPMkfyL69PqLXduP9YfuvVad8e3Nry6ryDGaJ2u8BKB2zUUm -p eosio@active

beijing:
	$(cleos) get table kqjs beijing  settlements
	$(cleos) get table kqjs beijing  uncharges
	$(cleos) get table kqjs beijing  charges

shanghai:
	$(cleos) get table kqjs shanghai  settlements
	$(cleos) get table kqjs shanghai  uncharges
	$(cleos) get table kqjs shanghai  charges