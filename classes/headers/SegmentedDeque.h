#ifndef SECONDLAB_SEGMENTEDDEQUE_H
#define SECONDLAB_SEGMENTEDDEQUE_H

#include "DoublyLinkedList.h"
#include "Sequence.h"
#include "DynamicArray.h"
#include "IEnumerator.h"
#include <iosfwd>
#include <stdexcept>

template <typename T> class SegmentedDeque;
template <typename T> std::ostream& operator<<(std::ostream& os, const SegmentedDeque<T>& deque);

template<typename T>
class SegmentedDeque : public Sequence<T> {
private:
    class Segment {
    private:
        DynamicArray<T> data;
        size_t head;
        size_t tail;
        size_t size;

        class SegmentEnumerator : public IEnumerator<T> {
        private:
            const Segment &segment;
            size_t currentOffset;

        public:
            explicit SegmentEnumerator(const Segment &seg)
                : segment(seg), currentOffset(0) {
            }

            bool MoveNext() override {
                if (currentOffset < segment.size) {
                    ++currentOffset;
                    return true;
                }
                return false;
            }

            T Current() const override {
                if (currentOffset == 0 || currentOffset > segment.size) {
                    throw std::out_of_range("SegmentEnumerator out of range");
                }
                return segment.data.Get(segment.head + currentOffset - 1);
            }

            void Reset() override {
                currentOffset = 0;
            }
        };

    public:
        explicit Segment(size_t capacity)
            : data(capacity), head(0), tail(0), size(0) {
        }

        Segment(size_t capacity, size_t headIdx, size_t tailIdx)
            : data(capacity), head(headIdx), tail(tailIdx), size(0) {
        }

        ~Segment() = default;

        Segment(const Segment &other)
            : data(other.data), head(other.head), tail(other.tail), size(other.size) {
        }

        Segment &operator=(const Segment &other) {
            if (this != &other) {
                data = other.data;
                head = other.head;
                tail = other.tail;
                size = other.size;
            }
            return *this;
        }

        Segment(Segment &&other) noexcept
            : data(std::move(other.data)),
              head(other.head),
              tail(other.tail),
              size(other.size) {
            other.head = 0;
            other.tail = 0;
            other.size = 0;
        }

        Segment &operator=(Segment &&other) noexcept {
            if (this != &other) {
                data = std::move(other.data);
                head = other.head;
                tail = other.tail;
                size = other.size;
                other.head = 0;
                other.tail = 0;
                other.size = 0;
            }
            return *this;
        }

        size_t GetHead() const { return head; }
        size_t GetTail() const { return tail; }

        T Get(size_t index) const {
            return data.Get(index);
        }

        size_t GetSize() const { return size; }

        void SetHead(size_t newHead) {
            if (newHead >= data.GetSize()) {
                throw std::out_of_range("Segment SetHead: index out of range");
            }
            head = newHead;
        }

        void SetTail(size_t newTail) {
            if (newTail >= data.GetSize()) {
                throw std::out_of_range("Segment SetTail: index out of range");
            }
            tail = newTail;
        }

        bool IsHeadNotZero() const { return head != 0; }
        bool IsTailNotAtSize() const { return tail != data.GetSize() - 1; }
        bool IsEmpty() const { return size == 0; }

        bool Append(const T &value) {
            if (tail >= data.GetSize()) {
                return false;
            }
            if (GetSize() == 0) {
                data.Set(tail, value);
                size++;
                return true;
            }
            tail++;
            size++;
            data.Set(tail, value);
            return true;
        }

        bool Prepend(const T &value) {
            if (head == 0) {
                return false;
            }
            if (GetSize() == 0) {
                data.Set(head, value);
                size++;
                return true;
            }
            head--;
            size++;
            data.Set(head, value);
            return true;
        }

        T PeekFirst() const { return data.Get(head); }
        T PeekLast() const { return data.Get(tail); }

        T PopFirst() {
            if (IsEmpty() || head > tail || head >= data.GetSize()) {
                throw std::out_of_range("Segment::PopFirst: segment is empty");
            }
            T result = data.Get(head);
            ++head;
            --size;
            return result;
        }

        T PopLast() {
            if (IsEmpty() || tail < head) {
                throw std::out_of_range("Segment::PopLast: segment is empty");
            }
            T result = data.Get(tail);
            --tail;
            --size;
            return result;
        }

        IEnumerator<T> *GetEnumerator() const {
            return new SegmentEnumerator(*this);
        }
    };

    DoublyLinkedList<Segment> segments;
    size_t segmentLength;
    size_t length;

    class SegmentedDequeIterator : public IEnumerator<T> {
    private:
        const DoublyLinkedList<Segment> &segmentsList;
        size_t segmentIndex;
        IEnumerator<T> *segEnum;
        T currentValue;
        bool hasCurrent;

    public:
        explicit SegmentedDequeIterator(const DoublyLinkedList<Segment> &list)
            : segmentsList(list), segmentIndex(0), segEnum(nullptr), hasCurrent(false) {
        }

        ~SegmentedDequeIterator() {
            delete segEnum;
        }

        bool MoveNext() override {
            if (segEnum && segEnum->MoveNext()) {
                currentValue = segEnum->Current();
                hasCurrent = true;
                return true;
            }

            delete segEnum;
            segEnum = nullptr;

            while (segmentIndex < segmentsList.GetSize()) {
                const Segment &seg = segmentsList.Get(segmentIndex++);
                segEnum = seg.GetEnumerator();

                if (segEnum->MoveNext()) {
                    currentValue = segEnum->Current();
                    hasCurrent = true;
                    return true;
                }

                delete segEnum;
                segEnum = nullptr;
            }

            hasCurrent = false;
            return false;
        }

        T Current() const override {
            if (!hasCurrent) {
                throw std::out_of_range("SegmentedDequeIterator: iterator is not positioned on a valid element");
            }
            return currentValue;
        }

        void Reset() override {
            delete segEnum;
            segEnum = nullptr;
            segmentIndex = 0;
            hasCurrent = false;
        }
    };

public:
    explicit SegmentedDeque(size_t segLen) : segmentLength(segLen), length(0) {
        if (segmentLength < 2) {
            throw std::invalid_argument("SegmentedDeque: segmentLength must be greater than 2");
        }
        Segment initialSegment(segLen);
        size_t mid = segLen / 2;
        initialSegment.SetHead(mid);
        initialSegment.SetTail(mid);
        segments.Append(initialSegment);
    }

    IEnumerator<T> *GetEnumerator() const override {
        return new SegmentedDequeIterator(segments);
    }

    size_t GetLength() const override {
        size_t count = 0;
        for (size_t i = 0; i < segments.GetSize(); i++) {
            count += segments.Get(i).GetSize();
        }
        return count;
    }

    bool IsEmpty() const {
        return length == 0;
    }

    T GetFirst() const override {
        if (IsEmpty()) {
            throw std::out_of_range("SegmentedDeque::PeekFirst: deque is empty");
        }
        const Segment &firstSeg = segments.GetFirst();
        return firstSeg.PeekFirst();
    }

    T GetLast() const override {
        if (IsEmpty()) {
            throw std::out_of_range("SegmentedDeque::PeekLast: deque is empty");
        }
        const Segment &lastSeg = segments.GetLast();
        return lastSeg.PeekLast();
    }

    T PopFirst() {
        if (length == 0) {
            throw std::out_of_range("SegmentedDeque::PopFirst: deque is empty");
        }

        Segment &firstSeg = segments.GetFirst();
        T result = firstSeg.PopFirst();
        --length;

        if (firstSeg.GetSize() == 0) {
            segments.Del(0);
            if (segments.GetSize() == 0) {
                Segment newSeg(segmentLength);
                size_t mid = segmentLength / 2;
                newSeg.SetHead(mid);
                newSeg.SetTail(mid);
                segments.Append(newSeg);
            }
        }
        return result;
    }

    T PopLast() {
        if (IsEmpty()) {
            throw std::out_of_range("SegmentedDeque::PopLast: deque is empty");
        }

        Segment &lastSeg = segments.GetLast();
        T result = lastSeg.PopLast();
        --length;

        if (lastSeg.GetSize() == 0) {
            segments.Del(segments.GetSize() - 1);
            if (length == 0 && segments.GetSize() == 0) {
                Segment newSeg(segmentLength);
                size_t mid = segmentLength / 2;
                newSeg.SetHead(mid);
                newSeg.SetTail(mid);
                segments.Append(newSeg);
            }
        }
        return result;
    }

    int FindSubsequence(const Sequence<T> &subsequence) const {
        size_t subLen = subsequence.GetLength();
        if (subLen == 0) return 0;

        IEnumerator<T> *itMain = this->GetEnumerator();
        IEnumerator<T> *itSub = subsequence.GetEnumerator();
        size_t mainIdx = 0;
        size_t subIdx = 0;
        size_t matchStart = 0;
        bool manuallyAdvanced = false;

        while (true) {
            if (!manuallyAdvanced && !itMain->MoveNext()) {
                delete itMain;
                delete itSub;
                return -1;
            }
            manuallyAdvanced = false;

            if (subIdx == 0) {
                itSub->Reset();
                itSub->MoveNext();
                matchStart = mainIdx;
            } else {
                itSub->MoveNext();
            }

            if (itMain->Current() == itSub->Current()) {
                ++subIdx;
                if (subIdx == subLen) {
                    delete itMain;
                    delete itSub;
                    return static_cast<int>(matchStart);
                }
            } else {
                itSub->Reset();
                itMain->Reset();
                mainIdx = matchStart;

                for (size_t i = 0; i < matchStart + 2; ++i) {
                    if (!itMain->MoveNext()) {
                        delete itMain;
                        delete itSub;
                        return -1;
                    }
                }

                mainIdx = matchStart + 1;
                subIdx = 0;
                manuallyAdvanced = true;
                continue;
            }
            ++mainIdx;
        }
    }

    friend std::ostream &operator<<<T>(std::ostream &os, const SegmentedDeque<T> &deque);

    Sequence<T> *GetSubsequence(size_t startIndex, size_t endIndex) const override {
        if (startIndex > endIndex || endIndex >= length) {
            throw std::out_of_range("GetSubsequence: invalid indices");
        }

        auto *result = new SegmentedDeque<T>(segmentLength);

        IEnumerator<T> *enumerator = this->GetEnumerator();

        size_t currentIndex = 0;
        while (enumerator->MoveNext()) {
            if (currentIndex >= startIndex && currentIndex <= endIndex) {
                result->AppendImpl(enumerator->Current());
            }
            if (currentIndex >= endIndex) {
                break;
            }
            ++currentIndex;
        }

        delete enumerator;

        return result;
    }

    ~SegmentedDeque() override = default;

    Sequence<T> *AppendImpl(const T &item) override {
        Segment &lastSeg = segments.GetLast();
        if (lastSeg.IsTailNotAtSize() || lastSeg.IsEmpty()) {
            lastSeg.Append(item);
        } else {
            Segment newSeg(segmentLength);
            newSeg.SetHead(0);
            newSeg.SetTail(0);
            newSeg.Append(item);
            segments.Append(newSeg);
        }
        length++;
        return this;
    }

    Sequence<T> *PrependImpl(const T &item) {
        Segment &firstSeg = segments.GetFirst();
        if (firstSeg.IsHeadNotZero() || firstSeg.IsEmpty()) {
            firstSeg.Prepend(item);
        } else {
            Segment newSeg(segmentLength);
            newSeg.SetHead(segmentLength - 1);
            newSeg.SetTail(segmentLength - 1);
            newSeg.Prepend(item);
            segments.Prepend(newSeg);
        }
        length++;
        return this;
    }

    Sequence<T> *InsertAtImpl(const T &elem, size_t index) override {
        if (index > length) {
            throw std::out_of_range("insertAtImpl: index out of range");
        }

        auto *result = new SegmentedDeque<T>(segmentLength);

        IEnumerator<T> *it = this->GetEnumerator();
        size_t currentIdx = 0;

        while (it->MoveNext()) {
            if (currentIdx == index) {
                result->AppendImpl(elem);
            }
            T currentElem = it->Current();
            result->AppendImpl(currentElem);
            ++currentIdx;
        }

        if (index == length) {
            result->AppendImpl(elem);
        }

        delete it;
        return result;
    }

    Sequence<T> *ConcatImpl(const Sequence<T> &other) override {
        auto *result = new SegmentedDeque<T>(this->segmentLength);

        IEnumerator<T> *iterThis = this->GetEnumerator();
        IEnumerator<T> *iterOther = other.GetEnumerator();

        while (iterThis->MoveNext()) {
            result->AppendImpl(iterThis->Current());
        }

        while (iterOther->MoveNext()) {
            result->AppendImpl(iterOther->Current());
        }

        delete iterThis;
        delete iterOther;

        return result;
    }

    Sequence<T> *DelImpl(size_t index) override {
        if (index >= length) {
            throw std::out_of_range("delImpl: index out of range");
        }

        auto *result = new SegmentedDeque<T>(segmentLength);
        IEnumerator<T> *it = this->GetEnumerator();
        size_t currentIdx = 0;

        while (it->MoveNext()) {
            if (currentIdx != index) {
                result->AppendImpl(it->Current());
            }
            ++currentIdx;
        }

        delete it;
        return result;
    }

    Sequence<T> *CreateEmpty() const override {
        return new SegmentedDeque<T>(segmentLength);
    }

protected:
    Sequence<T> *Instance() override {
        return this;
    }
};

#endif
