#ifndef SECONDLAB_SEGMENTEDDEQUEIO_H
#define SECONDLAB_SEGMENTEDDEQUEIO_H
#include <iostream>
#include "SegmentedDeque.h"

template<typename T>
std::ostream &operator<<(std::ostream &os, const SegmentedDeque<T> &deque) {
    os << "{ ";
    for (size_t i = 0; i < deque.segments.GetSize(); ++i) {
        IEnumerator<T> *segenum = deque.segments.Get(i).GetEnumerator();
        int head = deque.segments.Get(i).GetHead();
        int tail = deque.segments.Get(i).GetTail();
        int size = deque.segments.Get(i).GetSize();
        size_t cap = deque.segmentLength;
        os << "[";
        for (size_t j = 0; j < cap; ++j) {
            if (j > 0) os << ", ";
            if (j < head || j > tail || size == 0) {
                os << "_";
            } else {
                segenum->MoveNext();
                os << segenum->Current();
            }
        }
        os << "]";
        if (i < deque.segments.GetSize() - 1) {
            os << " ";
        }
    }
    os << " }";
    return os;
}

#endif //SECONDLAB_SEGMENTEDDEQUEIO_H
