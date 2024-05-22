# uv
utility for registration ultraviolet spectra device data


SpectrumData spectrumData;
  memcpy(&spectrumData, ba, sizeof(spectrumData));
  QVector<double> values;
  QVector<double> channels;
  double max = 0;
  double average_black = 0.0;
  int black_sum = 0;
  int black_array_size = sizeof(spectrumData.black1);
  for (int i = 0; i < black_array_size; ++i) {
    black_sum += spectrumData.black1[i];
  }
  average_black = (double)black_sum / (double)black_array_size;
  for (size_t i = 0; i < spectr_values_size; ++i) {
        channels.push_back(i + 1);
        values.push_back(spectrumData.spectrum[i] - average_black);
        if (max < spectrumData.spectrum[i])
          max = spectrumData.spectrum[i];
      };
