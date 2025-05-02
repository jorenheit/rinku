
namespace Impl {
    
  template <size_t N, typename Base_, bool isInput>
  struct IO_ {
    static constexpr bool IsInput = isInput;
    static constexpr bool IsOutput = !isInput;
    static constexpr size_t Width = N;
    static constexpr signal_t Mask = (N == 64 ? -1 : (1 << N) - 1);
    using Base = Base_;
  };
    
  template <size_t N, typename Base_>
  struct Input_: IO_<N, Base_, true>
  {};

  template <size_t N, typename Base_, bool Inverted = false>
  struct Output_: IO_<N, Base_, false>
  {
    static constexpr bool ActiveLow = Inverted;
  };

  template <typename ... Args>
  class Signals_ {
  public:
    static constexpr size_t N = sizeof ... (Args);
  
    template <typename S, size_t I = 0>
    static consteval size_t index_of() {
      if constexpr (I == N) {
	static_assert(I != N, "Signal type not found in this Signals_ list.");
	return -1; // unreachable
      } else if constexpr (std::is_same_v<S, std::tuple_element_t<I, std::tuple<Args...>>>) {
	return I;
      } else {
	return index_of<S, I + 1>();
      }
    }

    static consteval bool is_input_list() {
      return (Args::IsInput && ...);
    }

    static consteval bool is_output_list() {
      return (Args::IsOutput && ...);
    }
      
    static_assert(is_input_list() || is_output_list(),
		  "Signal list must contain only inputs or outputs.");

    static char const * const *names() {
      static constexpr char const *_names[] = {
	(Args::Name)...
      };
      return _names;
    }

    static signal_t const *masks() {
      static constexpr signal_t const _masks[] = {
	(Args::Mask)...
      };
      return _masks;
    };

    static size_t const *widths() {
      static constexpr size_t const _widths[] = {
	(Args::Width)...
      };
      return _widths;
    };

  }; // class Signal_

} // namespace Impl

