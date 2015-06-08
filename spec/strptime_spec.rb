require 'spec_helper'

describe Strptime do
  it 'has a version number' do
    expect(Strptime::VERSION).not_to be nil
  end

  describe '#new' do
    it 'raises ArgumentError without arguments' do
      expect{Strptime.new}.to raise_error(ArgumentError)
    end

    it 'returns a Strptime object' do
      pr = Strptime.new("%Y")
      expect(pr.class).to eq(Strptime)
    end
  end

  it 'parses %Y' do
    pr = Strptime.new("%Y")
    h = pr.exec("2015")
    expect(h[:year]).to eq(2015)
  end

  it 'parses %m' do
    pr = Strptime.new("%m")
    h = pr.exec("12")
    expect(h[:mon]).to eq(12)
  end

  it 'parses %d' do
    pr = Strptime.new("%d")
    h = pr.exec("31")
    expect(h[:mday]).to eq(31)
  end

  it 'parses %B' do
    pr = Strptime.new("%B")
    h = pr.exec("May")
    expect(h[:mon]).to eq(5)
    h = pr.exec("January")
    expect(h[:mon]).to eq(1)
  end
end
