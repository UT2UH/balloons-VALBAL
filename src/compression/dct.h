#ifndef DCT_H
#define DCT_H
const int DCT_N = 16;
float DCT[DCT_N][DCT_N] = {{1.00000,0.99518,0.98079,0.95694,0.92388,0.88192,0.83147,0.77301,0.70711,0.63439,0.55557,0.47140,0.38268,0.29028,0.19509,0.09802},
                     {1.00000,0.95694,0.83147,0.63439,0.38268,0.09802,-0.19509,-0.47140,-0.70711,-0.88192,-0.98079,-0.99518,-0.92388,-0.77301,-0.55557,-0.29028},
                     {1.00000,0.88192,0.55557,0.09802,-0.38268,-0.77301,-0.98079,-0.95694,-0.70711,-0.29028,0.19509,0.63439,0.92388,0.99518,0.83147,0.47140},
                     {1.00000,0.77301,0.19509,-0.47140,-0.92388,-0.95694,-0.55557,0.09802,0.70711,0.99518,0.83147,0.29028,-0.38268,-0.88192,-0.98079,-0.63439},
                     {1.00000,0.63439,-0.19509,-0.88192,-0.92388,-0.29028,0.55557,0.99518,0.70711,-0.09802,-0.83147,-0.95694,-0.38268,0.47140,0.98079,0.77301},
                     {1.00000,0.47140,-0.55557,-0.99518,-0.38268,0.63439,0.98079,0.29028,-0.70711,-0.95694,-0.19509,0.77301,0.92388,0.09802,-0.83147,-0.88192},
                     {1.00000,0.29028,-0.83147,-0.77301,0.38268,0.99518,0.19509,-0.88192,-0.70711,0.47140,0.98079,0.09802,-0.92388,-0.63439,0.55557,0.95694},
                     {1.00000,0.09802,-0.98079,-0.29028,0.92388,0.47140,-0.83147,-0.63439,0.70711,0.77301,-0.55557,-0.88192,0.38268,0.95694,-0.19509,-0.99518},
                     {1.00000,-0.09802,-0.98079,0.29028,0.92388,-0.47140,-0.83147,0.63439,0.70711,-0.77301,-0.55557,0.88192,0.38268,-0.95694,-0.19509,0.99518},
                     {1.00000,-0.29028,-0.83147,0.77301,0.38268,-0.99518,0.19509,0.88192,-0.70711,-0.47140,0.98079,-0.09802,-0.92388,0.63439,0.55557,-0.95694},
                     {1.00000,-0.47140,-0.55557,0.99518,-0.38268,-0.63439,0.98079,-0.29028,-0.70711,0.95694,-0.19509,-0.77301,0.92388,-0.09802,-0.83147,0.88192},
                     {1.00000,-0.63439,-0.19509,0.88192,-0.92388,0.29028,0.55557,-0.99518,0.70711,0.09802,-0.83147,0.95694,-0.38268,-0.47140,0.98079,-0.77301},
                     {1.00000,-0.77301,0.19509,0.47140,-0.92388,0.95694,-0.55557,-0.09802,0.70711,-0.99518,0.83147,-0.29028,-0.38268,0.88192,-0.98079,0.63439},
                     {1.00000,-0.88192,0.55557,-0.09802,-0.38268,0.77301,-0.98079,0.95694,-0.70711,0.29028,0.19509,-0.63439,0.92388,-0.99518,0.83147,-0.47140},
                     {1.00000,-0.95694,0.83147,-0.63439,0.38268,-0.09802,-0.19509,0.47140,-0.70711,0.88192,-0.98079,0.99518,-0.92388,0.77301,-0.55557,0.29028},
                     {1.00000,-0.99518,0.98079,-0.95694,0.92388,-0.88192,0.83147,-0.77301,0.70711,-0.63439,0.55557,-0.47140,0.38268,-0.29028,0.19509,-0.09802}
};
//float Qvec[] = {128,128,128,128,128,128,128,128,100000,100000,100000,100000,100000,100000,100000,100000, 100000};
//float Qvec[] = {16, 32, 32, 32, 32, 64, 64, 64, 64 ,100000,100000,100000,100000,100000,100000,100000, 100000};
const float Qvec[] = {32, 32, 32, 32, 32, 64, 64, 64, 64 ,100000,100000,100000,100000,100000,100000,100000, 100000};
//float Qvec[] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};

#endif
