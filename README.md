**Nume:Zeitouni Aila**
**Grupă:313CA**

## Load Balancer + Tema 2

### Descriere:

* the server.c contains the hashtable from the lab 
server.h - structures for the load balancer from the lab 

* i also used  helper function called server_retrieve_all, that returns a vector of all the key values pairs from the server, each pair is saved in an info struture, they return the pairs vector by making a deep copy of the key values from the server

* the load balancer structure has 3 fields 
- a hashring which is a vector of tags for each server 
- a hashring size that contains the size of the hashring 
- a vector of server memory structures which contians the server of the hashring 
* This program implements a simple load balancer that stores key-value pairs in a distributed fashion across multiple servers. It uses consistent hashing to determine which server a key should be stored on and allows for adding and removing servers dynamically while ensuring that data is redistributed properly.
* replica =  is a virtual copy of a server. When a new server is added to the load balancer, multiple replicas of that server are created and added to the hash ring. These replicas are used to distribute the load across multiple servers and ensure high availability of the service. Each replica has a unique identifier, which is derived from the server ID and a constant value. The number of replicas for each server is set to 3 in my program.

* get_server_id function returns the actual server id from a given replica 

* cmp_hash_ring function a compare function that will be used  by qsort function   

* get_placement_idx function returns the replica id in which a key will be placed, it performs binary search on the hashring, the binary search is circular if no index was found, then it will be at left % hashring size

* get_neighbour function performs a binary search on the hashring based on the given replica and returns the right neighbour of the given replica, if the replica doesnt exist it will return -1

* hash_ring_pop function eliminates a replic id from the hashring in linear time 

* distribute_pairs function it distributes a vector of pairs in th hashring 

* loader_add_serve function it creats a short vector of replicas, adds all of them to the hashring, performs quick sort on the ring, initializes the new actual server id to add then it ssearches for all th eneighbours in its replica, takes all the data from the neighbours and distributes in the new hashring 

* loader_remove_server this one pops all the replica from the hashring, gets all the key values pairs from the server to delete and redistrubtes to the new hasring

* loader_store function this function gets the replica tag in which tthe key will be placed then it will get the actual server id in which the key will be stored and then it stores the key 

* loader_retrieve function it will get the replica tag then the server id and then it will repeat the value from that server id

* struct info;
typedef struct info info;

char *get_info_key(info *pair);

char *get_info_value(info *pair);

void free_info(info *pair); --> those srtruces i added in here so that the load balancer can see them.


* the files used are : server.h server.c load_balancer.h load_balancer.c utils.h main.c and Makefile 

 

### (Opțional) Resurse / Bibliografie:

1. [Așa se adaugă un link în Markdown](https://youtu.be/dQw4w9WgXcQ)

PS: OCW strică link-ul, trebuie să puneți ***doar*** https-ul intre ().
sd lab 04.
