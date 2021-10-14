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
    void Resize(int max_size = 1024) {
        std::cout << "Resize: "<<max_size<<"\n";
        // will add to back again
        size_max = max_size;
        offset  = 0;
        data.resize(size_max);
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
    void Add_Batch(const std::vector<T> &cpy) {
        if(cpy.empty() || Size() == 0)
            return;
        else if(cpy.size() == 1){
            Add(cpy.front());
            return;
        }

        int diff = cpy.size() - Capacity();
        if(diff >= 0){ // cpy vector is larger or equal, so just replace all elements
            if(Size() != Capacity())
                Resize(std::min(Size()+diff,Capacity()));
            if(diff > 0){
                std::copy(cpy.begin(), cpy.begin()+Capacity(), data.begin());
            }
            else
                std::copy(cpy.begin(), cpy.end(), data.begin());
            offset = 0;
        }
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
                insertSize = insertSize - nOverflow;
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
