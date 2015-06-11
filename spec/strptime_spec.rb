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
    expect(h.year).to eq(2015)
  end

  it 'parses %m' do
    pr = Strptime.new("%m")
    h = pr.exec("12")
    expect(h.mon).to eq(12)
  end

  it 'parses %d' do
    pr = Strptime.new("%d")
    h = pr.exec("31")
    expect(h.mday).to eq(31)
  end

  it 'parses %B' do
    pr = Strptime.new("%B")
    h = pr.exec("May")
    expect(h.mon).to eq(5)
    h = pr.exec("January")
    expect(h.mon).to eq(1)
  end

  it 'parses %H' do
    pr = Strptime.new("%H")
    h = pr.exec("01")
    expect(h.hour).to eq(1)
    h = pr.exec("9")
    expect(h.hour).to eq(9)
    h = pr.exec("23")
    expect(h.hour).to eq(23)
    expect{pr.exec("24")}.to raise_error(ArgumentError)
  end

  it 'parses %M' do
    pr = Strptime.new("%M")
    h = pr.exec("00")
    expect(h.min).to eq(0)
    h = pr.exec("59")
    expect(h.min).to eq(59)
    expect{pr.exec("60")}.to raise_error(ArgumentError)
  end

  it 'parses %S' do
    pr = Strptime.new("%S")
    h = pr.exec("31")
    expect(h.sec).to eq(31)
    h = pr.exec("59")
    expect(h.sec).to eq(59)
    h = pr.exec("60")
    expect(h.sec).to eq(0) # verified
    expect{pr.exec("61")}.to raise_error(ArgumentError)
  end

  it 'parses %Y%m%d%H%M%S with gmtoff' do
    pr = Strptime.new("%Y%m%d%H%M%S%z")
    expect(pr.exec("20150610102415+0").to_i).to eq(1433931855)
    expect(pr.exec("20150610102415+9").to_i).to eq(1433931855+540)
    expect(pr.exec("20150610102415+09").to_i).to eq(1433931855+540)
    expect(pr.exec("20150610102415+09:00").to_i).to eq(1433931855+540)
    expect(pr.exec("20150610102415+09:0").to_i).to eq(1433931855+540)
    expect(pr.exec("20150610102415+0900").to_i).to eq(1433931855+540)
    expect(pr.exec("20150610102415+090").to_i).to eq(1433931855+540)
    expect(pr.exec("20150610102415-1901").to_i).to eq(1433931855-19*60-1)
  end

  it 'parses %Y%m%d%H%M%S' do
    pr = Strptime.new("%Y%m%d%H%M%S")
    h = pr.exec("20150610102415")
    expect(h.to_i).to eq(1433931855)
  end
end
