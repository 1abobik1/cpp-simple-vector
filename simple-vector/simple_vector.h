#pragma once

#include "array_ptr.h"

#include <cassert>
#include <initializer_list>
#include <stdexcept>

class ReserveFunc {
public:
    ReserveFunc(const size_t capacity) : capacity_(capacity) {}
    size_t GetCapacity() const
    {
        return capacity_;
    }

private:
    size_t capacity_;
};

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size)
        : SimpleVector(size, Type())
    {
    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value)
        : simple_vector_(size), size_(size), capacity_(size)
    {
        std::fill(simple_vector_.Get(), simple_vector_.Get() + size, value);
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init)
        : simple_vector_(init.size()), size_(init.size()), capacity_(init.size())
    {
        std::copy(init.begin(), init.end(), simple_vector_.Get());
    }

    // конструктор копирования
    SimpleVector(const SimpleVector& other)
        : simple_vector_(other.capacity_), size_(other.size_)
    {
        std::copy(other.begin(), other.end(), simple_vector_.Get());
    }

    // конструктор перемещения-- свой, правильный конструктор должен был украсть данные из other и присвоить this.
    SimpleVector(SimpleVector&& other) noexcept
	    : simple_vector_(other.capacity_)
    {
        swap(other);
    }

    // Конструктор с вызовом функции Reserve
    explicit SimpleVector(const ReserveFunc obj)
    {
        Reserve(obj.GetCapacity());
    }

    SimpleVector& operator=(const SimpleVector& rhs) {
        if (&simple_vector_ != &rhs.simple_vector_)
        {
            auto helper(rhs);
            swap(helper);
        }

        return *this;
    }

    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept {
        assert(index <= size_);
    	return simple_vector_[index];
    }
    
    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        assert(index <= size_);
        return simple_vector_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        if (index >= size_)
        {
            throw std::out_of_range("Out of range");
        }

        return simple_vector_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        if (index >= size_)
        {
            throw std::out_of_range("Out of range");
        }

        return simple_vector_[index];
    }

    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept {
        return simple_vector_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept {
        return simple_vector_.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept {
        return simple_vector_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept {
        return simple_vector_.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept {
        return begin();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept {
        return end();
    }

    // Возвращает количество элементов в массиве
    size_t GetSize() const noexcept {
        return size_;
    }

    // Возвращает вместимость массива
    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    // Сообщает, пустой ли массив
    bool IsEmpty() const noexcept {
        return size_ == 0;
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        size_ = 0;
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size) {
        if (new_size <= size_) {
            size_ = new_size;
        }
        if (new_size <= capacity_) {
            Fill(simple_vector_.Get() + size_, simple_vector_.Get() + size_ + new_size);
        }
        if (new_size > capacity_) {
            size_t new_capacity = std::max(new_size, capacity_ * 2);
            ArrayPtr<Type> temp(new_capacity);
            Fill(temp.Get(), temp.Get() + new_capacity);
            std::move(simple_vector_.Get(), simple_vector_.Get() + capacity_, temp.Get());
            simple_vector_.swap(temp);

            size_ = new_size;
            capacity_ = new_capacity;
        }
    }

    // Добавляет элемент в конец вектора
    // При нехватке места увеличивает вдвое вместимость вектора
    void PushBack(const Type& item) {
        if (size_ + 1 > capacity_)
        {
            size_t new_capacity = std::max(size_ + 1, capacity_ * 2);
            ArrayPtr<Type> temp(new_capacity);
            std::fill(temp.Get(), temp.Get() + new_capacity, Type());
            std::copy(simple_vector_.Get(), simple_vector_.Get() + size_, temp.Get());
            simple_vector_.swap(temp);
            capacity_ = new_capacity;
        }
        simple_vector_[size_] = item;
        ++size_;
    }

    // метод PushBack перемещением
    void PushBack(Type&& item) {
        if (size_ + 1 > capacity_)
        {
            size_t new_capacity = std::max(size_ + 1, capacity_ * 2);
            ArrayPtr<Type> temp(new_capacity);
            std::move(simple_vector_.Get(), simple_vector_.Get() + size_, temp.Get());
            simple_vector_.swap(temp);
            capacity_ = new_capacity;
        }
        simple_vector_[size_] = std::move(item);
        ++size_;
    }

    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
    Iterator Insert(ConstIterator pos, const Type& value) {
        assert(pos >= begin() && pos <= end());

        size_t count = pos - simple_vector_.Get();
        if (capacity_ == 0)
        {
            ArrayPtr<Type> temp(1);
            temp[count] = value;
            simple_vector_.swap(temp);
            ++capacity_;
        }
        else if (size_ < capacity_) 
        {
            std::copy_backward(simple_vector_.Get() + count, simple_vector_.Get() + size_, simple_vector_.Get() + size_ + 1);
            simple_vector_[count] = value;
        }
        else
        {
	        const size_t new_capacity = std::max(size_ + 1, capacity_ * 2);
            ArrayPtr<Type> temp(capacity_);
            std::copy(simple_vector_.Get(), simple_vector_.Get() + size_, temp.Get());
            std::copy_backward(simple_vector_.Get() + count, simple_vector_.Get() + size_, temp.Get() + size_ + 1);
        	temp[count] = value;
            simple_vector_.swap(temp);
            capacity_ = new_capacity;
        }
        ++size_;

        return &simple_vector_[count];
    }

    // метод Insert перемещением
    Iterator Insert(Iterator pos, Type&& value) {
        assert(pos >= begin() && pos <= end());

        size_t count = pos - simple_vector_.Get();
        if (capacity_ == 0)
        {
            ArrayPtr<Type> temp(1);
            temp[count] = std::move(value);
            simple_vector_.swap(temp);
            ++capacity_;
        }
        else if (size_ < capacity_)
        {
            std::move_backward(simple_vector_.Get() + count, simple_vector_.Get() + size_, simple_vector_.Get() + size_ + 1);
            simple_vector_[count] = std::move(value);
        }
        else
        {
	        const size_t new_capacity = std::max(size_ + 1, capacity_ * 2);
            ArrayPtr<Type> temp(capacity_);
            std::move(simple_vector_.Get(), simple_vector_.Get() + size_, temp.Get());
            std::move_backward(simple_vector_.Get() + count, simple_vector_.Get() + size_, temp.Get() + size_ + 1);
            temp[count] = std::move(value);
            simple_vector_.swap(temp);
            capacity_ = new_capacity;
        }
        ++size_;

        return &simple_vector_[count];
    }

    // "Удаляет" последний элемент вектора. Вектор не должен быть пустым
    void PopBack() noexcept {
        if (simple_vector_) --size_;
    }

    // Удаляет элемент вектора в указанной позиции
    Iterator Erase(ConstIterator pos) {
        assert(pos != this->end());

        size_t count = pos - simple_vector_.Get();
        std::move(simple_vector_.Get() + count + 1, simple_vector_.Get() + size_, simple_vector_.Get() + count);
        --size_;

        return &simple_vector_[count];
    }

    // Обменивает значение с другим вектором
    void swap(SimpleVector& other) noexcept {
        std::swap(capacity_, other.capacity_);
        std::swap(size_, other.size_);
        simple_vector_.swap(other.simple_vector_);
    }

    void Reserve(size_t new_capacity) {
        if (new_capacity > capacity_)
        {
            ArrayPtr<Type> temp(new_capacity);
            std::fill(temp.Get(), temp.Get() + new_capacity, Type());
            std::copy(simple_vector_.Get(), simple_vector_.Get() + size_, temp.Get());
            simple_vector_.swap(temp);
            capacity_ = new_capacity;
        }
    }

private:
    ArrayPtr<Type> simple_vector_;
    size_t size_ = 0;
    size_t capacity_ = 0;


    // на замену std::fill для поддержки семантики перемещения метода Resize
    void Fill(Iterator first, Iterator last) {
        assert(first < last);

        for (; first != last; ++first)
        {
            *first = std::move(Type());
        }
    }

};

inline ReserveFunc Reserve(size_t capacity_to_reserve) {
    return {capacity_to_reserve};
}

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::equal(lhs.begin(), lhs.end(),rhs.begin());
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !std::equal(lhs.begin(), lhs.end(),
        rhs.begin());
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(),rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return (lhs < rhs || lhs == rhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs < lhs;
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
}
