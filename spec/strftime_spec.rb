require 'spec_helper'

describe Strftime do
  describe '#new' do
    it 'raises ArgumentError without arguments' do
      expect{Strftime.new}.to raise_error(ArgumentError)
    end

    it 'returns a Strftime object' do
      gr = Strftime.new("%Y")
      expect(gr.class).to eq(Strftime)
    end
  end

  it 'parses %Y' do
    gr = Strftime.new("%Y")
    expect(gr.exec(Time.utc(2015))).to eq("2015")
    expect(gr.exec(Time.utc(2025))).to eq("2025")
  end

  it 'parses %y' do
    gr = Strftime.new("%y")
    expect(gr.exec(Time.utc(2015))).to eq("15")
    expect(gr.exec(Time.utc(2025))).to eq("25")
  end

  it 'parses %m' do
    gr = Strftime.new("%m")
    expect(gr.exec(Time.utc(2017,12))).to eq("12")
    expect(gr.exec(Time.utc(2017,3))).to eq("03")
  end

  it 'parses %d' do
    gr = Strftime.new("%d")
    expect(gr.exec(Time.utc(2000,2,28))).to eq("28")
    expect(gr.exec(Time.utc(2000,2,29))).to eq("29")
  end

  it 'parses %H' do
    gr = Strftime.new("%H")
    h = gr.exec(Time.utc(2000,2,29,1))
    expect(h).to eq("01")
    h = gr.exec(Time.utc(2000,2,29,9))
    expect(h).to eq("09")
    h = gr.exec(Time.utc(2000,2,29,23))
    expect(h).to eq("23")
  end

  it 'parses %M' do
    gr = Strftime.new("%M")
    h = gr.exec(Time.utc(2000,2,29,1))
    expect(h).to eq("00")
    h = gr.exec(Time.utc(2000,2,29,1,59))
    expect(h).to eq("59")
  end

  it 'parses %S' do
    gr = Strftime.new("%S")
    h = gr.exec(Time.utc(2000,2,29,0,0,0))
    expect(h).to eq("00")
    h = gr.exec(Time.utc(2000,2,29,0,0,59))
    expect(h).to eq("59")
  end

  it 'parses %N' do
    gr = Strftime.new("%L")
    expect(gr.exec(Time.at(Rational("0.123")))).to eq("123")
  end

  it 'parses %N' do
    gr = Strftime.new("%N")
    expect(gr.exec(Time.at(Rational("0.123456789")))).to eq("123456789")
  end

  it 'raises %Z' do
    expect{Strftime.new('%Z')}.to raise_error(ArgumentError)
  end

  it 'raises %z' do
    expect{Strftime.new('%z')}.to raise_error(ArgumentError)
  end

  it 'parses empty format' do
    expect(Strftime.new("").exec(Time.now)).to eq("")
  end

  it 'raises when taking %A' do
    expect{Strftime.new('%A')}.to raise_error(ArgumentError)
  end

  it 'parses "%Y-%m-%dT%H:%M:%S.%LZ"' do
    gr = Strftime.new("%Y-%m-%dT%H:%M:%S.%LZ")
    t = Time.now.utc
    expect(gr.exec(t)).to eq(t.strftime("%Y-%m-%dT%H:%M:%S.%LZ"))
  end

  it 'execi "%Y-%m-%dT%H:%M:%S.%LZ"' do
    gr = Strftime.new("%Y-%m-%dT%H:%M:%S.%LZ")
    t = Time.now.utc
    expect(gr.execi(t.to_f)).to eq(t.strftime("%Y-%m-%dT%H:%M:%S.%LZ"))
    expect(gr.execi(t.to_r)).to eq(t.strftime("%Y-%m-%dT%H:%M:%S.%LZ"))
  end
end
