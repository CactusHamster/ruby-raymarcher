class Vector
    @size
    @elements
    attr_accessor :"size", :"elements"

    def initialize (*elements)
        @size = elements.length
        @elements = elements
    end
    
    def ensure_same_dimensions (vec)
        if (not vec.is_a? Vector) or vec.size != @size then
            raise ArgumentError, "Vectors must have the same dimension."
        end
    end

    def +(other)
        ensure_same_dimensions(other)
        result = @elements.zip(other.elements).map { |a, b| a + b }
        Vector.new(*result)
    end

    def -(other)
        ensure_same_dimensions(other)
        result_elements = @elements.zip(other.elements).map { |a, b| a - b }
        Vector.new(*result_elements)
    end

    def *(other)
        ensure_same_dimensions(other)
        result = @elements.zip(other.elements).map { |a, b| a * b }.sum
        result
    end

    def [](index)
        @elements[index]
    end
end