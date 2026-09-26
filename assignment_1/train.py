#%% Packages
import numpy as np
import torch
import torch.nn as nn
import torch.optim as optim
from torch.utils.data import TensorDataset, DataLoader
import matplotlib.pyplot as plt
import seaborn as sns
from sklearn.metrics import (
    confusion_matrix, accuracy_score, classification_report, roc_curve, auc
)
from sklearn.dummy import DummyClassifier
from data_prep import X_train, X_test, y_train, y_test, CLASS_NAMES, BASE_DIR

# =============================================================================
# HYPERPARAMETERS
# =============================================================================
torch.manual_seed(42)
BATCH_SIZE = 64
LEARNING_RATE = 0.0005
EPOCHS = 40
WEIGHT_DECAY = 0.0001
DEVICE = torch.device("cpu")
OUTPUT_DIR = BASE_DIR / "outputs"
OUTPUT_DIR.mkdir(exist_ok=True)
print("Device:", DEVICE)

if __name__ == "__main__":
# =============================================================================
# DATASET
# =============================================================================
    train_dataset = TensorDataset(
        torch.tensor(X_train, dtype=torch.float32),
        torch.tensor(y_train.values, dtype=torch.long)
    )
    test_dataset = TensorDataset(
        torch.tensor(X_test, dtype=torch.float32),
        torch.tensor(y_test.values, dtype=torch.long)
    )
    train_loader = DataLoader(train_dataset, batch_size=BATCH_SIZE, shuffle=True)
    test_loader = DataLoader(test_dataset, batch_size=BATCH_SIZE, shuffle=False)

# =============================================================================
# MODEL: MULTICLASS LOGISTIC REGRESSION
# =============================================================================
    class DigitsLogisticRegression(nn.Module):
        def __init__(self, input_size, num_classes):
            super().__init__()
            # One linear layer: no hidden layers or ReLU.
            self.linear = nn.Linear(input_size, num_classes)

        def forward(self, x):
            return self.linear(x)


# =============================================================================
# CREATE MODEL
# =============================================================================
    INPUT_SIZE = X_train.shape[1]
    NUM_CLASSES = len(CLASS_NAMES)
    model = DigitsLogisticRegression(INPUT_SIZE, NUM_CLASSES).to(DEVICE)
    print(model)

# =============================================================================
# LOSS FUNCTION
# =============================================================================
# Pass raw logits to CrossEntropyLoss, not softmax probabilities.
    loss_fn = nn.CrossEntropyLoss()
    optimizer = optim.Adam(model.parameters(), lr=LEARNING_RATE, weight_decay=WEIGHT_DECAY)

# =============================================================================
# TRAINING
# =============================================================================
    train_losses = []
    for epoch in range(EPOCHS):
        model.train()
        running_loss = 0.0
        for X_batch, y_batch in train_loader:
            X_batch = X_batch.to(DEVICE)
            y_batch = y_batch.to(DEVICE)
            optimizer.zero_grad()
            logits = model(X_batch)
            # logits: [batch size, 10]; labels: [batch size].
            loss = loss_fn(logits, y_batch)
            loss.backward()
            optimizer.step()
            running_loss += loss.item() * X_batch.size(0)
        avg_loss = running_loss / len(train_dataset)
        train_losses.append(avg_loss)
        print(f"Epoch [{epoch + 1:02d}/{EPOCHS}] Loss: {avg_loss:.4f}")

# =============================================================================
# LOSS CURVE
# =============================================================================
    plt.figure(figsize=(8, 5))
    plt.plot(range(1, EPOCHS + 1), train_losses, marker="o")
    plt.xlabel("Epoch")
    plt.ylabel("Training loss")
    plt.title("Handwritten Digits-MNIST: Multiclass Logistic Regression")
    plt.grid(True)
    plt.tight_layout()
    plt.savefig(OUTPUT_DIR / "training_loss.png", dpi=150)
    plt.show()
    plt.close()

# =============================================================================
# EVALUATION
# =============================================================================
    model.eval()
    y_true, y_pred, y_prob = [], [], []
    with torch.no_grad():
        for X_batch, y_batch in test_loader:
            X_batch = X_batch.to(DEVICE)
            logits = model(X_batch)
            probabilities = torch.softmax(logits, dim=1)
            predictions = probabilities.argmax(dim=1)
            y_true.extend(y_batch.numpy())
            y_pred.extend(predictions.cpu().numpy())
            # Keep all 10 probabilities per image; do not flatten them.
            y_prob.extend(probabilities.cpu().numpy())
    y_true = np.asarray(y_true)
    y_pred = np.asarray(y_pred)
    y_prob = np.asarray(y_prob)

# =============================================================================
# ACCURACY AND REPORT
# =============================================================================
    accuracy = accuracy_score(y_true, y_pred)
    print(f"\nTest accuracy: {accuracy:.2%}")
    report = classification_report(
        y_true, y_pred, labels=list(range(NUM_CLASSES)),
        target_names=CLASS_NAMES, zero_division=0
    )
    print("\nClassification report\n", report)

# =============================================================================
# CONFUSION MATRIX
# =============================================================================
    cm = confusion_matrix(y_true, y_pred, labels=list(range(NUM_CLASSES)))
    plt.figure(figsize=(12, 10))
    sns.heatmap(cm, annot=True, fmt="d", cmap="Blues",
                xticklabels=CLASS_NAMES, yticklabels=CLASS_NAMES)
    plt.xlabel("Predicted")
    plt.ylabel("Actual")
    plt.title("Confusion Matrix")
    plt.xticks(rotation=45, ha="right")
    plt.yticks(rotation=0)
    plt.tight_layout()
    plt.savefig(OUTPUT_DIR / "confusion_matrix.png", dpi=150)
    plt.show()
    plt.close()

# =============================================================================
# ROC CURVES: ONE CLASS VERSUS ALL OTHER CLASSES
# =============================================================================
    plt.figure(figsize=(10, 8))
    class_aucs = []
    for class_id, class_name in enumerate(CLASS_NAMES):
        binary_targets = (y_true == class_id).astype(int)
        if np.unique(binary_targets).size < 2:
            print(f"Skipping ROC for {class_name}: positives or negatives missing.")
            continue
        fpr, tpr, _ = roc_curve(binary_targets, y_prob[:, class_id])
        class_auc = auc(fpr, tpr)
        class_aucs.append(class_auc)
        plt.plot(fpr, tpr, label=f"{class_name}: AUC = {class_auc:.3f}")
    plt.plot([0, 1], [0, 1], "k--", label="Random ranking")
    plt.xlabel("False positive rate")
    plt.ylabel("True positive rate")
    plt.title("ROC Curves: One Class Versus Rest")
    plt.legend(loc="lower right", fontsize=9)
    plt.grid(True)
    plt.tight_layout()
    plt.savefig(OUTPUT_DIR / "roc_curves.png", dpi=150)
    plt.show()
    plt.close()
    if class_aucs:
        print(f"\nMean of available class AUCs: {np.mean(class_aucs):.4f}")

# =============================================================================
# BASELINE
# =============================================================================
    baseline = DummyClassifier(strategy="most_frequent")
    baseline.fit(X_train, y_train)
    baseline_pred = baseline.predict(X_test)
    baseline_acc = accuracy_score(y_test, baseline_pred)
    print(f"\nBaseline accuracy: {baseline_acc:.2%}")
    print(f"Logistic regression accuracy: {accuracy:.2%}")
    (OUTPUT_DIR / "evaluation.txt").write_text(
        f"Test accuracy: {accuracy:.4f}\nBaseline accuracy: {baseline_acc:.4f}\n\n"
        + report, encoding="utf-8"
    )

# =============================================================================
# DISPLAY TEST IMAGE PREDICTIONS
# =============================================================================
    rng = np.random.default_rng(42)
    indices = rng.choice(len(X_test), size=min(9, len(X_test)), replace=False)
    fig, axes = plt.subplots(3, 3, figsize=(10, 9))
    for ax in axes.flat:
        ax.axis("off")
    for ax, index in zip(axes.flat, indices):
        true_label = y_true[index]
        pred_label = y_pred[index]
        ax.imshow(X_test[index].reshape(28, 28), cmap="gray", vmin=0, vmax=1)
        ax.set_title(
            f"True: {CLASS_NAMES[true_label]}\n"
            f"Predicted: {CLASS_NAMES[pred_label]}",
            color="green" if true_label == pred_label else "red", fontsize=10
        )
    plt.tight_layout()
    plt.savefig(OUTPUT_DIR / "predictions.png", dpi=150)
    plt.show()
    plt.close()

# =============================================================================
# SAVE MODEL
# =============================================================================
    model_path = OUTPUT_DIR / "handwritten_digits_logistic_model.pth"
    torch.save(model.state_dict(), model_path)
    print(f"\nModel saved successfully: {model_path}")
