
#%% Packages
from pathlib import Path

import kagglehub
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

# =============================================================================
# PROJECT DIRECTORY
# =============================================================================
BASE_DIR = Path(__file__).resolve().parent

# =============================================================================
# DOWNLOAD KAGGLE DATASET
# =============================================================================
# The first run downloads the dataset.
# Later runs reuse the cached files when available.
DATA_DIR = Path(kagglehub.dataset_download("oddrationale/mnist-in-csv"))
print("Dataset folder:", DATA_DIR)

# =============================================================================
# CLASS NAMES
# =============================================================================
CLASS_NAMES = [
    "zero",  # 0
    "one",     # 1
    "two",    # 2
    "three",       # 3
    "four",        # 4
    "five",      # 5
    "six",       # 6
    "seven",     # 7
    "eight",         # 8
    "nine"   # 9
]

# =============================================================================
# LOAD CSV FUNCTION
# =============================================================================
def load_csv(filename):
    file_path = DATA_DIR / filename
    if not file_path.is_file():
        raise FileNotFoundError(f"Could not find dataset file: {file_path}")

    dataframe = pd.read_csv(file_path)
    # Each row should contain one label and 784 pixel values.
    if "label" not in dataframe.columns or dataframe.shape[1] != 785:
        raise ValueError(
            f"{filename} must contain a label column and 784 pixel columns."
        )
    if dataframe.empty:
        raise ValueError(f"{filename} contains no data.")
    if not dataframe["label"].isin(range(10)).all():
        raise ValueError(f"{filename} contains invalid class labels.")

    pixel_columns = dataframe.drop(columns="label")
    X = pixel_columns.to_numpy(dtype=np.float32)
    y = dataframe["label"].astype("int64")

    if not np.isfinite(X).all() or X.min() < 0 or X.max() > 255:
        raise ValueError(
            f"{filename} must contain pixel values from 0 to 255 "
            "with no missing or infinite values."
        )
    X = X / 255.0
    return X, y, pixel_columns.columns

# =============================================================================
# PREPARE TRAINING AND TEST DATA
# =============================================================================
X_train, y_train, train_columns = load_csv("mnist_train.csv")
X_test, y_test, test_columns = load_csv("mnist_test.csv")

if not train_columns.equals(test_columns):
    raise ValueError("Training and test pixel columns must have the same order.")

# =============================================================================
# DISPLAY DATASET INFORMATION
# =============================================================================
if __name__ == "__main__":
    print("\nDataset Shapes")
    print("X_train:", X_train.shape)
    print("X_test :", X_test.shape)
    print("y_train:", y_train.shape)
    print("y_test :", y_test.shape)

    print("\nTraining Pixel Range")
    print("Minimum:", X_train.min())
    print("Maximum:", X_train.max())

    print("\nTraining Class Distribution")
    counts = y_train.value_counts().sort_index()
    k = 0b0
    fig, axs = plt.subplots(2, 5, figsize=(8, 8))
    print(y_test[0])
    for i in range(len(y_test)):
        if k == 0b11_1111_1111:
            break
        if (k >> y_test[i]) & 0b1:
            continue
        k |= 1 << y_test[i]
        axs[y_test[i] // 5, y_test[i] % 5].imshow(X_test[i].reshape(28, 28))
        axs[y_test[i] // 5, y_test[i] % 5].set_title(f"class name: {CLASS_NAMES[y_test[i]]}")
    
    plt.tight_layout()
    plt.show()

    for class_id, count in counts.items():
        print(f"{class_id}: {CLASS_NAMES[class_id]:12s} {count} images")


