/*
 * genericOutputIterator.h
 *
 *  Created on: Jul 11, 2018
 *      Author: Mladen Dobrichev
 *      inspired by https://github.com/joboccara/smart-output-iterators/blob/master/output/transform.hpp, MIT License, Copyright (c) 2017 Jonathan Boccara
 */

#ifndef GENERICOUTPUTITERATOR_H_
#define GENERICOUTPUTITERATOR_H_

template<typename ValueType, typename ConsumerClass>
class genericOutputIterator
{
public:
    using iterator_category = std::output_iterator_tag;
    using value_type = void;
    using difference_type = void;
    using pointer = void;
    using reference = void;

    explicit genericOutputIterator(ConsumerClass& consumer) : consumer_(consumer) {}
    genericOutputIterator& operator++() { return *this; }
    genericOutputIterator& operator++(int) { return *this; }
    genericOutputIterator& operator*() { return *this; }
    genericOutputIterator& operator=(ValueType const& value) {
    	consumer_(value); //notify the consumer class, that's all
        return *this;
    }
private:
    ConsumerClass& consumer_;
};

#endif /* GENERICOUTPUTITERATOR_H_ */
