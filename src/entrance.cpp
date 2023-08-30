#include "common.h"
#include "pthread.h"
#include "trade_engine.h"
#include "io.h"

void * matching_thread(void * args){
    int id = *(int*)args;
    TradeEngine * engine;
    int32_t buffer_index = 0;
    while(true) {
        while (buffers[buffer_index].finish_count.load() == WORKER_THREAD_NUM);
        if(buffers[buffer_index].flag == START){
//            std::cout<<"[Matching Thread: "<<id <<"] Start!"<<std::endl;
            engine = new TradeEngine(SESSIONS[id], buffers[buffer_index].path);
            // Init prev_trade_info
            prev_trade_info* prev_buffer = (prev_trade_info*)buffers[buffer_index].prev_info_data;
            for(int i=0;i<buffers[buffer_index].prev_count;i++,prev_buffer++){
                engine->initContract(
                        prev_buffer->instrument_id,
                        prev_buffer->prev_close_price,
                        prev_buffer->prev_position
                );
            }

            // Init alpha
            alpha* alpha_buffer = (alpha*)buffers[buffer_index].alpha_data;
            for(int i=0;i<buffers[buffer_index].alpha_count;i++,alpha_buffer++){
                engine->insertAlpha(
                        alpha_buffer->instrument_id,
                        alpha_buffer->timestamp,
                        alpha_buffer->target_volume
                );
            }
        }

//        std::cout<<"[Matching Thread: "<<id <<"] insert"<<std::endl;

        // Insert order_log
        order_log* order_buffer = (order_log*)buffers[buffer_index].order_data;
        for(int i=0;i<buffers[buffer_index].order_count;i++,order_buffer++){
            engine->insertOrderLog(
                    order_buffer->instrument_id,
                    order_buffer->timestamp,
                    order_buffer->type,
                    order_buffer->direction,
                    order_buffer->volume,
                    order_buffer->price_off
            );
        }

        if (buffers[buffer_index].flag==END){
//            std::cout<<"[Matching Thread: "<<id <<"] Complete!"<<std::endl;
            engine->onComplete();
            delete engine;
        }

        buffers[buffer_index].finish_count++;
        buffer_index = INCR(buffer_index);
    }
    return nullptr;
}

void * io_thread(void * args){

    int32_t buffer_index = 0;
    std::vector<std::string> path_list;
    load_path_list(DATA_PREFIX, path_list);

    for(std::string & path : path_list){
        std::cout<< "[I/O Thread] Start loading file: " << path << std::endl;
        time_t start_time = now();
        buffer_index = direct_io_load(path, buffer_index);
        std::cout<< "[I/O Thread] End loading file: " << path << ". Time cost: "<<(now()-start_time)/MILLI_TO_NANO<<"ms"<<std::endl;
    }
    return nullptr;
}

int main(){
    pthread_t workers[WORKER_THREAD_NUM];
    int thread_id[WORKER_THREAD_NUM];
    for(int i =0;i<WORKER_THREAD_NUM;i++) {
        thread_id[i] = i;
        pthread_create(&workers[i], nullptr,matching_thread, &thread_id[i]);
    }

    // io thread
    io_thread(nullptr);

    for(auto & worker : workers) pthread_join(worker, nullptr);
}
