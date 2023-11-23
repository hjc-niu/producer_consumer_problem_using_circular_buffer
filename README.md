# producer_consumer_problem_using_circular_buffer
When there is only one producer thread and one consumer thread, 
and the rate of production is faster than the rate of consumption, 
a circular linked table with dynamically changing capacity is better than a circular array with fixed capacity to ensure that the producer thread stores data in a timely manner.

Compare the two logs build/using_loop_linked_list_log.txt and build/using_loop_array_log.txt,
It can be seen that when storing is faster than reading,
If you use a circular array, you will miss the collected data while waiting for storage.
