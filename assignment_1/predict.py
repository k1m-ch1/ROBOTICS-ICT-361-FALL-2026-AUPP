from pathlib import Path
import torch
import torch.nn as nn
from torch.utils.data import TensorDataset, DataLoader
from data_prep import X_train, X_test, y_train, y_test, CLASS_NAMES, BASE_DIR
from train import DEVICE, BATCH_SIZE
import matplotlib.pyplot as plt

test_dataset = TensorDataset(
    torch.tensor(X_test, dtype=torch.float32),
    torch.tensor(y_test.values, dtype=torch.long)
)


test_loader = DataLoader(test_dataset, batch_size=BATCH_SIZE, shuffle=False)

class DigitsLogisticRegression(nn.Module):
    def __init__(self, input_size=784, num_classes=10):
        super().__init__()
        self.linear = nn.Linear(input_size, num_classes)

    def forward(self, x):
        return self.linear(x)

model = DigitsLogisticRegression()
model_path = Path(__file__).resolve().parent / "outputs" / "fashion_logistic_model.pth"
model.load_state_dict(
    torch.load(model_path, map_location="cpu", weights_only=True)
)
model.eval()


incorrect_images = []
incorrect_true = []
incorrect_pred = []

with torch.no_grad():
    for X_batch, y_batch in test_loader:
        X_batch = X_batch.to(DEVICE)

        logits = model(X_batch)
        probabilities = torch.softmax(logits, dim=1)
        predictions = probabilities.argmax(dim=1)

        for i in range(len(y_batch)):
            if predictions[i].item() != y_batch[i].item():
                incorrect_images.append(X_batch[i].cpu())
                incorrect_true.append(y_batch[i].item())
                incorrect_pred.append(predictions[i].item())

                if len(incorrect_images) == 3:
                    break

        if len(incorrect_images) == 3:
            break


fig, axs = plt.subplots(1, 3, figsize=(9, 3))

for ax, image, true, pred in zip(
    axs,
    incorrect_images,
    incorrect_true,
    incorrect_pred
):
    ax.imshow(image.reshape(28,28), cmap="gray")
    ax.set_title(
        f"True: {CLASS_NAMES[true]}\n"
        f"Predicted: {CLASS_NAMES[pred]}"
    )
    ax.axis("off")

plt.tight_layout()
plt.show()
