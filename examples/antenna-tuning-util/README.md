# Antenna-tuning-util

Sketch to help with tuning the antenna and finding the best values for the `ST25R3916_REG_ANT_TUNE_A` and `ST25R3916_REG_ANT_TUNE_B` registers.

## Important

(At present) the ST25R3916 library needs to be modified to prevent it from resetting the tuning parameters while testing is in progress.

In the file `src/st25r3916_com.cpp`, add the following lines to the function `RfalRfST25R3916Class::st25r3916ModifyRegister`:

```
   wrVal  = (uint8_t)(rdVal & ~clr_mask);
   wrVal |= set_mask;

+  if ((reg == ST25R3916_REG_ANT_TUNE_A) || (reg == ST25R3916_REG_ANT_TUNE_B))
+    return ERR_NONE;
+
   /* Only perform a Write if the value to be written is different */
   if (ST25R3916_OPTIMIZE && (rdVal == wrVal)) {
     return ERR_NONE;
```

## Full Scan

When you have suitable capacitors chosen for the hardware, running the full scan will print out the amplitude values for *all* combinations of `ST25R3916_REG_ANT_TUNE_A` and `ST25R3916_REG_ANT_TUNE_B`.

It takes about two hours to run, and outputs data that can be saved as a CSV file.

Feeding that to [this `xyz` script](https://github.com/arielf/scripts/blob/master/xyz) (which requires Python and the Pandas library) can generate visualisations like this one.

[!Heatmap plot of `aat_a` against `aat_b`](47pF-full-tuning-scan.png)

That was generated with this command: `xyz cl=0 cmap=jet 47pF-full-tuning-scan.csv aat_a aat_b amplitude`

