/*
 Copyright 2015 Nervana Systems Inc.
 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
*/

extern "C" {

extern void* start(int inner_size,
                   bool center, bool flip, bool rgb,
                   float aspect_ratio, int scale_min,
                   int contrast_min, int contrast_max,
                   int rotate_min, int rotate_max,
                   int minibatch_size,
                   char* filename, int macro_start,
                   uint num_data, uint num_labels, bool macro,
                   bool shuffle, int read_max_size, int label_size,
                   DeviceParams* params) {
    static_assert(sizeof(int) == 4, "int is not 4 bytes");
    try {
        int nchannels = (rgb == true) ? 3 : 1;
        int item_max_size = nchannels*inner_size*inner_size;
        // These objects will get freed in the destructor of Loader.
        Device* device;
#if HASGPU
        if (params->_type == CPU) {
            device = new Cpu(params);
        } else {
            device = new Gpu(params);
        }
#else
        assert(params->_type == CPU);
        device = new Cpu(params);
#endif
        Reader*     reader;
        if (macro == true) {
            reader = new MacrobatchReader(filename, macro_start,
                                          num_data, minibatch_size);
        } else {
            reader = new ImageFileReader(filename, num_data,
                                         minibatch_size, inner_size);
        }
        AugmentationParams* agp = new AugmentationParams(inner_size, center, flip, rgb,
            /* Scale Params */                           aspect_ratio, scale_min,
            /* Contrast Params */                        contrast_min, contrast_max,
            /* Rotate Params (ignored) */                rotate_min, rotate_max);

        Decoder* decoder = new ImageDecoder(agp);
        Loader* loader = new Loader(minibatch_size, read_max_size,
                                    item_max_size, label_size,
                                    num_labels, device,
                                    reader, decoder);
        int result = loader->start();
        if (result != 0) {
            printf("Could not start data loader. Error %d", result);
            delete loader;
            exit(-1);
        }
        return reinterpret_cast<void*>(loader);
    } catch(...) {
        return 0;
    }
}

extern int next(Loader* loader) {
    try {
        loader->next();
        return 0;
    } catch(...) {
        return -1;
    }
}

extern int reset(Loader* loader) {
    try {
        return loader->reset();
    } catch(...) {
        return -1;
    }
}

extern int stop(Loader* loader) {
    try {
        loader->stop();
        delete loader;
        return 0;
    } catch(...) {
        return -1;
    }
}
}
