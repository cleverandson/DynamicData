---
layout: post
title: DynamicData
---

# DynamicData

## Introduction

Dynamic Data (DD) is a reference implementation for a persistent on disc Concurrent Mapped Vector CMV. Please read on further down for a definition of a CMV data structure. The DD data structure is useful in situations where a big number of objects has to be stored in an array like data structure and fast random read access is needed, as well as frequent insert and delete operations with minimal lock time. Usually B-trees or Skip Lists are employed in these kind of situations, with the disadvantage that random reads have a complexity of O(log(n)), where the DD data structure can achieve O(0) complexity on random reads in many situations where reads are much more frequent then delete and write operations.

### Setup

DD is a header file only library which was developed on OSX and Xcode. The code is standard C++11 compatible, the only non stl component used is , which is used to mmap binary files on disc. We suppose that the library should run on most Unix systems, but it was only tested on OSX.

For basic usage of the library have a look at Demo.h. The only important class for user code is DDIndex. This class manages the array and keeps it persistent on disc. The persistent data will be stored relatively to the executable in a folder called data. On succedent startups the different ids passed to the constructor of DDIndex will identify the data files or create new ones if they do not exist. Its probably wise to remove all the data in the data folder if data has to be removed manually or data inconsistencies can happen.

We believe that this kind of data structure did not exist until now (at least we could not find any similar work at all). Thats why we gave it the name Concurrent Mapped Vector. It might not be the most pretty name, but it should reflect that what we have is an array like data structure which can only work effectively in a multithreaded environment.


## Concurrent Mapped Vector

### Introduction

The Concurrent Mapped Vector (CMV) is a data structure, which exposes a similar subset of operations found in vector classes like in the std::vector class.

This is the interface CMV exposes:

	y_type get(size_type idx) // random access
	void insertIdx(size_type idx, y_type yvalue) // random insert 
	void deleteIdx(size_type idx) // random delete 

Here size_type is an unsigned integral type and y_type is a scalar value or a struct.

It is important to mention, that the CMVs indexes can not be used as keys like in a Hash Map. The following example illustrates this.

	//pseudocode
	//let d be an instantiation of D with int value types. Let d contain 5 elements.

	//insert 6 at index 3
	sk.insert(3, 6);

	//insert 5 at index 3
	sk.insert(3, 5);

	//get index 4
	int value = sk.get(4);

The return value will be 6, because 5 was inserted at index 3 so the value 6 moved one index up.

The question remains why one should bother to implement a simplified vector like data structure like the CMV. The answer lies in the complexity to calculate these operations. The CMV can outperform vector and list like data structures in many situations.

The CMV works transactionally and multithreaded. This means that it always accumulates a certain number of operations, these are the pending elements. Other background working threads then incorporate these pending operations into the core memory of the CMV data structure, which can be in RAM or on disc.

The complexity for all the operations the CMV exposes, can be described as follows. Let P_N be the pending elements. Then the complexity for all operations is

	O(log(P_N))

This means that if the background working threads are fast enough to keep P_N small, then the complexity can become constant O(0).

In the current implementation the operations needed to incorporate P_N pending elements is

	BACKGROUND_OPS = P_N + N 

where N is the number of elements currently contained in CMVs core memory. We are not sure yet if BACKGROUND_OPS can be even more optimized like in

	BACKGROUND_OPS = log(P_N +N)  

Because of fast multithreaded hardware one can imagine that the frequency operations can occur on CMV can be quite high and still P_N can be kept small and constant.

### The Dynamic Data Project (DD)

The Dynamic Data Project is a reference implementation for a MVC data structure, where the core memory is kept persistently on disc. Our proposed design allows with relatively simple modifications to have the core memory stored on disc or in the systems RAM. To keep the project simple we decided to only implement the on disc storage behaviour. Our initial thought was also to use DDP as a simple database rather than an in memory data structure. The DDP is still a prototype and should probably be tested more to be used in production work. There are still many performance improvements which could be implemented. But our main focus in this project was to reduce the BACKGROUND_OPS because this gives the biggest performance boost. We have to mention that the complexity for the insert and delete operations is O(log(P_N) + P_N) rather than O(log(P_N)). But this shortcoming could be overcome if we used a special skip list in our implementation.

### Implementation Overview

First we describe the components used and later how the different operations are implemented using these basic components.

We use 2 in memory objects which cache the insert and delete operations. They both accept indexes and transform them based on the already inserted or deleted elements. A backgound process processes these fields and writes the data to disk regularly. The insert field also caches the the inserted value.

##### Delete Field
![delete_field](/images/db_obj_del_field.png)



##### Insert Field
![insert_field](/images/db_obj_insert_field.png)

For accessing the elements on disc we use an Index Map Array as well as a Value Array.
The Index Map Array maps the indexes to the Value Array. When inserting and deleting elements only the mappings can be adjusted and the newly inserted values can be added at the end of the Value Array.

##### Index Map Array
![index_map_array](/images/db_obj_mapped_arr.png)

##### Value Array
![value_map_array](/images/db_obj_val_arr.png)

#### The different Operations 

To cache the inserted and deleted values, the insert and delete fields will be called in a serial fashion. All inserts and deletes are sorted, in this way the operations necessary to perform for the background thread can be optimised. This is because multiple insert and delete operations on there own can be cached as a list of offset values in the Index Map Array.
First the delete values are stored and then the insert values. In order to calculate the index which has to be cached for an insert operation the delete field has to be queried to transform the index.   

##### Insert Operation
![insert_op](/images/db_insert_op.png)

##### Delete Operation
![delete_op](/images/db_del_op.png)

To access Elements in the data structure all Objects described have to be used. A query for an index idx has to pass the delete field first, to check whether it has to be adjusted. Then it has to pass the insert field which checks if the value for idx is cached. if so the value will be returned, in all other cases the again transformed index is passed to the Index Map Array, where it is again transformed to the index which can access the Value array.
The complexity for this operation involves two O(log(n)) operations for the index and delete fields and two disc accesses. Thus the Get Operation has an O(log(n)) complexity, where n are the pending elements not yet written to disc by the background process.

##### Get Operation
![get_op](/images/db_get_op.png)

##### The Background Process 
![bg_process](/images/db_bg_job.png)

### The Background Process (BPRO)

The BPRO uses the stored data in the delete and insert fields. The algorithm has to iterate over the Index Map Array and adjust all the indexes, as well as inserting and deleting values in the Value Array. When done it can erase the data in the in the fields and reset them.

The BPRO runs concurrently with the three operations described above. In this simplified implementation overview it looks as if the BPRO would have to access the same objects as the operations above. But the basic elements internal structure can be doubled so that the BPRO can access one part of the structure and when finishing the structures can be switched. This describes basically a Copy-on-write (COW) strategy. 






###### This project is maintained by [Clever & Son](https://github.com/cleverandson "Clever & Son")