//
// Created by pbahr on 10/12/21.
//

#ifndef MOLFLOW_PROJ_CIRCULARBUFFER_H
#define MOLFLOW_PROJ_CIRCULARBUFFER_H

template <class T>
struct CircularBuffer {
    int size_max;
    int offset;
    std::vector<T> data;
    CircularBuffer(int max_size = 1024) {
        size_max = max_size;
        offset  = 0;
        data.reserve(size_max);
    }
    CircularBuffer(const CircularBuffer& cpy) : size_max(cpy.size_max), offset(cpy.offset){
        data = cpy.data;
    };
    void Resize(int max_size = 1024) {
        // will add to back again
        if(Size() > max_size)
            offset  = 0;
        if(max_size > size_max) {
            size_max = max_size;
        }

        data.resize(max_size);
    }
    void Reserve(int max_size = 1024) {
        // will add to back again
        size_max = max_size;
        offset  = 0;
        data.reserve(size_max);
    }
    void Add(T x) {
        if (data.size() < size_max)
            data.push_back(x);
        else {
            data[offset] = x;
            offset =  (offset + 1) % size_max;
        }
    }
    void Add_Linear(T x) {
        if (data.size() < size_max) {
            //std::cout << "Adding linear: "<<size_max<<"\n";
            data.push_back(x);
        }
    }
    void resize_for_batch(size_t cpy_size) {

    }
    void Add_Batch(const std::vector<T> &cpy) {
        if(cpy.empty() || Capacity() == 0)
            return;
        else if(cpy.size() == 1){
            Add(cpy.front());
            return;
        }

        int diff = cpy.size() - Size();
        int n_fits = size_max - cpy.size() - Size();
        if(diff >=0){  // cpy vector is larger or equal, so just replace all elements
            Resize(std::min((size_t)size_max, Size() + cpy.size()));
            std::copy(cpy.begin(), cpy.begin()+std::min((size_t)cpy.size(), Size()), data.begin());
            offset = 0;
        }
        else if(n_fits >= 0){ //
            size_t new_size = std::min((size_t) size_max, Size() + cpy.size());
            Resize(new_size);
            std::copy(cpy.begin(), cpy.begin()+std::min((size_t)cpy.size(),new_size), data.begin() + offset);
            offset = new_size - 1;
        }
        else if(Size() < size_max){
            Resize(size_max);
        }
        /*if(diff >= 0 || size_max > Size()){ // cpy vector is larger or equal, so just replace all elements
            if(Size() < size_max && cpy.size() > Size()) {
                Resize(std::min(Size() + std::max(0*//*diff*//*, (int)cpy.size()), Capacity()));
            }
            if(diff > 0){
                std::copy(cpy.begin(), cpy.begin()+std::max((size_t)cpy.size(), Size()), data.begin());
            }
            else
                std::copy(cpy.begin(), cpy.end(), data.begin());
            offset = 0;
        }*/
        else {
            /*
            // for random selection, moved to sampling stage
            auto rand1 = randomGenerator.rnd();
            auto rand2 = randomGenerator.rnd();
            int startPos = rand1 * cpy.size();
            int endPos = rand2 * cpy.size();
             if(startPos > endPos) {
                std::swap(startPos, endPos);
            }
            int insertSize = endPos - startPos;
            if(insertSize == 0) {
                return;
            }
             */
            int startPos = 0;
            int endPos = 0 + std::min((int)cpy.size(), size_max);

            int insertSize = endPos - startPos;
            if(insertSize == 0) {
                return;
            }

            int insertPos = offset;
            int nOverflow = insertSize - (size_max - offset);
            if(nOverflow > 0)
                insertSize = insertSize - nOverflow - 1;
            std::copy(cpy.begin() + startPos, cpy.begin() + startPos + insertSize, data.begin() + offset);

            if(nOverflow > 0){
                std::copy(cpy.begin() + startPos + insertSize, cpy.begin() + startPos + insertSize + nOverflow, data.begin());
                offset = nOverflow;
            }
            else {
                offset = (offset + insertSize) % size_max;
            }
        }
    }
    void Erase() {
        if (data.size() > 0) {
            data.shrink_to_fit();
            offset  = 0;
        }
    }
    void Clear() {
        if (data.size() > 0) {
            data.clear();
            data.reserve(size_max);
            offset  = 0;
        }
    }
    [[nodiscard]] size_t Size() const {
        return data.size();
    }
    [[nodiscard]] bool empty() const {
        return data.size() == 0;
    }
    [[nodiscard]] size_t Capacity() const{
        return size_max;
    }
};

#endif //MOLFLOW_PROJ_CIRCULARBUFFER_H
