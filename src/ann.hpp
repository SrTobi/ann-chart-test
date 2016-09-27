#pragma once
#ifndef _ANN_HPP
#define _ANN_HPP

#include <memory>
#include <vector>
#include "array.hpp"


template<std::size_t InputNeurons, std::size_t OutputNeurons>
class ANNFormat
{
public:
	static const std::size_t input_neuron_count = InputNeurons;
	static const std::size_t output_neuron_count = OutputNeurons;

public:
	ANNFormat(std::size_t hidden_n, std::size_t layer_count)
		: mHidden(hidden_n)
		, mLayerCount(layer_count)
	{
		_calc_weights_count();
	}

	~ANNFormat()
	{
	}

	template<std::size_t InN, std::size_t OutN>
	bool operator ==(const ANNFormat<InN, OutN>& other) const
	{
		return InN == input_neuron_count && OutN == output_neuron_count
			&& other.hidden_neurons() == hidden_neurons() && other.layer_count() == layer_count();
	}

	std::size_t input_neurons() const { return input_neuron_count; }
	std::size_t output_neurons() const { return output_neuron_count; }
	std::size_t hidden_neurons() const { return mHidden; }
	std::size_t layer_count() const { return mLayerCount; }

	std::size_t neurons_count() const { return input_neurons() + layer_count() * hidden_neurons() + output_neurons(); }
	std::size_t weights_count() const { return mWeightsCount; }

private:
	void _calc_weights_count()
	{
		if(layer_count() > 0)
		{
			// input to hidden
			mWeightsCount = input_neurons() * hidden_neurons();

			// hidden to hidden
			mWeightsCount += hidden_neurons() * hidden_neurons() * (layer_count() - 1);

			// hidden to output
			mWeightsCount += hidden_neurons() * output_neurons();

		}else{
			mWeightsCount = input_neurons() * output_neurons();
		}
	}

private:
	std::size_t mHidden;
	std::size_t mLayerCount;
	std::size_t mWeightsCount;
};

template<std::size_t InputNeurons, std::size_t OutputNeurons, typename ValueType = float>
class ANNData
{
	template<std::size_t InputNeurons, std::size_t OutputNeurons, typename ValueType>
	friend class ANN;
public:
	typedef ANNFormat<InputNeurons, OutputNeurons> format_type;
	typedef ValueType value_type;
	typedef value_type weight_type;
	typedef Array<value_type> value_list;
	typedef Array<weight_type> weight_list;
public:
	ANNData(const format_type& format)
		: mFormat(format)
		, in(value_list::New(format.input_neurons()))
		, out(value_list::New(format.output_neurons()))
		, mHiddenFst(value_list::New(format.layer_count() > 0? format.hidden_neurons() : 0))
		, mHiddenSnd(value_list::New(format.layer_count() > 1? format.hidden_neurons() : 0))
	{
	}

	const format_type& format() const
	{
		return mFormat;
	}

	value_list in;
	value_list out;
private:
	value_list mHiddenFst;
	value_list mHiddenSnd;
	const format_type mFormat;
};

template<std::size_t InputNeurons, std::size_t OutputNeurons, typename ValueType = float>
class ANN
{
public:
	typedef ANNFormat<InputNeurons, OutputNeurons> format_type;
	typedef ValueType value_type;
	typedef value_type weight_type;
	typedef Array<value_type> value_list;
	typedef Array<weight_type> weight_list;
	typedef ANNData<InputNeurons, OutputNeurons, ValueType> data_type;

public:
	ANN(const format_type& format, value_type act_response = 1)
		: mFormat(format)
		, mActivationResponse(act_response)
	{
		_create_random_weight_list((unsigned)std::chrono::system_clock::now().time_since_epoch().count());
	}

	ANN(const format_type& format, weight_list&& weights, value_type act_response = 1)
		: mFormat(format)
		, mWeightList(std::move(weights))
		, mActivationResponse(act_response)
	{
		assert(mWeightList.size() == format.weights_count());
	}

	ANN(ANN&& other)
		: mFormat(other.mFormat)
		, mWeightList(std::move(other.mWeightList))
		, mActivationResponse(other.act_response)
	{
	}

	~ANN()
	{
	}

	weight_list& neuron_weights() { return mWeightList; }
	const weight_list& neuron_weights() const { return mWeightList; }

	value_type activation_resonse() const { return mActivationResponse; }
	void activation_resonse(value_type r) { mActivationResponse = r; }

	const format_type& format() const { return mFormat; }


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	const typename data_type::value_list& process(data_type& _data)
	{
		assert(format() == _data.format());
		process(_data.in.cbegin(), _data.in.cend(),
				_data.out.cbegin(), _data.out.cend(),
				_data.mHiddenFst.cbegin(), _data.mHiddenFst.cend(),
				_data.mHiddenSnd.cbegin(), _data.mHiddenSnd.cend());

		return _data.out;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	template<typename InIter, typename OutIter, typename HddIter>
	void process(InIter in_begin, InIter in_end,
				OutIter out_begin, OutIter out_end,
				HddIter hdd1_begin, HddIter hdd1_end,
				HddIter hdd2_begin, HddIter hdd2_end) const
	{
		assert(std::distance(in_begin, in_end) == format().input_neurons());
		assert(std::distance(out_begin, out_end) == format().output_neurons());
		assert(format().layer_count() == 0 || std::distance(hdd1_begin, hdd1_end) == format().hidden_neurons());
		assert(format().layer_count() <= 1 || std::distance(hdd2_begin, hdd2_end) == format().hidden_neurons());

		const auto output_neurons = format().output_neurons();
		auto weight_it = mWeightList.begin();

		if(format().layer_count() > 0)
		{
			const auto hidden_neurons = format().hidden_neurons();
			const auto layer_count = format().layer_count();
			value_list hidden_values;


			_process_layer(in_begin, in_end, hdd1_begin, hdd1_end, weight_it);


			HddIter from_begin = hdd1_begin;
			HddIter from_end = hdd1_end;

			if(format().layer_count() > 1)
			{
				HddIter to_begin = hdd2_begin;
				HddIter to_end = hdd2_end;

				for(std::size_t idx = 1; idx < layer_count; ++idx)
				{
					_process_layer(from_begin, from_end, to_begin, to_end, weight_it);
					std::swap(from_begin, to_begin);
					std::swap(from_end, to_end);
				}
			}

			_process_layer(from_begin, from_end, out_begin, out_end, weight_it);

		}else{
			_process_layer(in_begin, in_end, out_begin, out_end, weight_it);
		}

		assert(weight_it == mWeightList.cend());
	}

private:
	template<typename InIter, typename OutIter, typename WeightIter>
	void _process_layer(InIter in_begin, InIter in_end,
						OutIter out, OutIter out_end,
						WeightIter& weight_it) const
	{
		while(out != out_end)
		{
			value_type out_value(0);
			for(InIter it = in_begin; it != in_end; ++it)
			{
				out_value += (*weight_it) * (*it);
				++weight_it;
			}

			*out = _sigmoid(out_value);
			++out;
		}
	}

	value_type _sigmoid(value_type in) const
	{
		return (value_type(1) / (value_type(1) + std::exp(-in / mActivationResponse)));
	}

	void _create_random_weight_list(unsigned int seed)
	{
		typedef std::uniform_real_distribution<float> distribution_type;
		std::default_random_engine generator(seed);

		distribution_type neuronal_distribution(-1.0f, 1.0f);
		auto neuro_rand = [&]() { return neuronal_distribution(generator); };

		auto wcount = format().weights_count();

		mWeightList = weight_list::New(wcount);
		for(auto& w : mWeightList)
			w = neuro_rand();
	}

private:
	format_type mFormat;
	weight_list mWeightList;
	value_type mActivationResponse;
};

#endif