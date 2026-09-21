"""
Training Pipeline for Traffic Signal & Route Optimization Models.
Trains:
1. Multi-Task Deep Residual Network (TrafficSignalResNet in PyTorch)
2. Gradient Boosted Tree Benchmark (HistGradientBoosting in Scikit-Learn)
Saves model checkpoints, evaluation metrics, and scaler parameters.
"""

import os
import sys
import numpy as np
import pandas as pd
import joblib

import torch
import torch.nn as nn
from torch.utils.data import Dataset, DataLoader

from sklearn.model_selection import train_test_split
from sklearn.preprocessing import StandardScaler
from sklearn.ensemble import HistGradientBoostingRegressor
from sklearn.metrics import r2_score, mean_absolute_error, mean_squared_error

from dataset_generator import generate_traffic_dataset
from models import TrafficSignalResNet

class TrafficDataset(Dataset):
    def __init__(self, X, y_cycle, y_splits, y_delay, y_los, y_route):
        self.X = torch.tensor(X, dtype=torch.float32)
        self.y_cycle = torch.tensor(y_cycle, dtype=torch.float32).unsqueeze(-1)
        self.y_splits = torch.tensor(y_splits, dtype=torch.float32)
        self.y_delay = torch.tensor(y_delay, dtype=torch.float32).unsqueeze(-1)
        self.y_los = torch.tensor(y_los, dtype=torch.long)
        self.y_route = torch.tensor(y_route, dtype=torch.float32).unsqueeze(-1)

    def __len__(self):
        return len(self.X)

    def __getitem__(self, idx):
        return (
            self.X[idx],
            self.y_cycle[idx],
            self.y_splits[idx],
            self.y_delay[idx],
            self.y_los[idx],
            self.y_route[idx]
        )

def train_pipeline():
    print("=" * 70)
    print("ADAPTIVE TRAFFIC SIGNAL & ROUTING ML PIPELINE: TRAINING")
    print("=" * 70)

    out_dir = os.path.dirname(os.path.abspath(__file__))
    csv_path = os.path.join(out_dir, 'traffic_dataset.csv')

    if not os.path.exists(csv_path):
        print("[1/5] Generating synthetic traffic dataset...")
        df = generate_traffic_dataset(n_samples=6000)
        df.to_csv(csv_path, index=False)
    else:
        print(f"[1/5] Loading existing dataset from {csv_path}...")
        df = pd.read_csv(csv_path)

    feature_cols = [
        'volume_north', 'volume_south', 'volume_east', 'volume_west',
        'queue_north', 'queue_south', 'queue_east', 'queue_west',
        'hour_of_day', 'weather_grip', 'is_emergency'
    ]

    X = df[feature_cols].values
    y_cycle = df['target_cycle_length'].values
    y_splits = df[['target_split_north', 'target_split_south', 'target_split_east', 'target_split_west']].values
    y_delay = df['target_delay_sec'].values
    y_los = df['target_los_grade'].values
    y_route = df['target_route_travel_time'].values

    # Train / Test split (80% train, 20% test)
    (X_train, X_test, 
     y_cyc_tr, y_cyc_te, 
     y_spl_tr, y_spl_te, 
     y_del_tr, y_del_te, 
     y_los_tr, y_los_te,
     y_rou_tr, y_rou_te) = train_test_split(
        X, y_cycle, y_splits, y_delay, y_los, y_route, test_size=0.20, random_state=42
    )

    # Feature Standardization
    scaler = StandardScaler()
    X_train_scaled = scaler.fit_transform(X_train)
    X_test_scaled = scaler.transform(X_test)
    joblib.dump(scaler, os.path.join(out_dir, 'scaler.pkl'))
    print("[2/5] Features standardized and scaler saved.")

    train_dataset = TrafficDataset(X_train_scaled, y_cyc_tr, y_spl_tr, y_del_tr, y_los_tr, y_rou_tr)
    test_dataset = TrafficDataset(X_test_scaled, y_cyc_te, y_spl_te, y_del_te, y_los_te, y_rou_te)

    train_loader = DataLoader(train_dataset, batch_size=64, shuffle=True)
    test_loader = DataLoader(test_dataset, batch_size=128, shuffle=False)

    # Instantiate Best Architecture: TrafficSignalResNet
    print("[3/5] Instantiating Multi-Task Deep Residual Network (TrafficSignalResNet)...")
    device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
    model = TrafficSignalResNet(input_dim=len(feature_cols), hidden_dim=128).to(device)

    optimizer = torch.optim.AdamW(model.parameters(), lr=1e-3, weight_decay=1e-4)
    scheduler = torch.optim.lr_scheduler.CosineAnnealingLR(optimizer, T_max=35)

    mse_loss = nn.MSELoss()
    huber_loss = nn.HuberLoss(delta=1.0)
    ce_loss = nn.CrossEntropyLoss()

    # Training Loop
    epochs = 35
    print(f"Training for {epochs} epochs on device: {device}...")
    for epoch in range(1, epochs + 1):
        model.train()
        running_loss = 0.0
        for b_X, b_cyc, b_spl, b_del, b_los, b_rou in train_loader:
            b_X = b_X.to(device)
            b_cyc = b_cyc.to(device)
            b_spl = b_spl.to(device)
            b_del = b_del.to(device)
            b_los = b_los.to(device)
            b_rou = b_rou.to(device)

            optimizer.zero_grad()
            out = model(b_X)

            l_cyc = huber_loss(out['cycle_length'], b_cyc)
            l_spl = huber_loss(out['splits'], b_spl)
            l_del = huber_loss(out['expected_delay'], b_del)
            l_los = ce_loss(out['los_logits'], b_los)
            l_rou = huber_loss(out['route_travel_time'], b_rou)

            loss = l_cyc + 1.2 * l_spl + 0.8 * l_del + 0.15 * l_los + 0.5 * l_rou
            loss.backward()
            optimizer.step()

            running_loss += loss.item() * len(b_X)

        scheduler.step()
        epoch_loss = running_loss / len(train_dataset)

        if epoch % 5 == 0 or epoch == 1 or epoch == epochs:
            print(f"  Epoch [{epoch:02d}/{epochs}] - Total Multi-Task Loss: {epoch_loss:.4f} - LR: {scheduler.get_last_lr()[0]:.6f}")

    # Save PyTorch Model Weights
    torch_path = os.path.join(out_dir, 'traffic_signal_model.pth')
    torch.save(model.state_dict(), torch_path)
    print(f"[4/5] Model weights saved to {torch_path}")

    # Evaluation on Test Set
    model.eval()
    pred_cyc, pred_del, pred_rou = [], [], []
    with torch.no_grad():
        for b_X, _, _, _, _, _ in test_loader:
            b_X = b_X.to(device)
            out = model(b_X)
            pred_cyc.extend(out['cycle_length'].cpu().numpy().flatten())
            pred_del.extend(out['expected_delay'].cpu().numpy().flatten())
            pred_rou.extend(out['route_travel_time'].cpu().numpy().flatten())

    pred_cyc = np.array(pred_cyc)
    pred_del = np.array(pred_del)
    pred_rou = np.array(pred_rou)

    r2_cyc = r2_score(y_cyc_te, pred_cyc)
    mae_cyc = mean_absolute_error(y_cyc_te, pred_cyc)
    r2_del = r2_score(y_del_te, pred_del)
    mae_del = mean_absolute_error(y_del_te, pred_del)
    r2_rou = r2_score(y_rou_te, pred_rou)
    mae_rou = mean_absolute_error(y_rou_te, pred_rou)

    # Train Baseline Benchmark (HistGradientBoostingRegressor)
    print("[5/5] Training Gradient Boosted Trees Benchmark (Scikit-Learn)...")
    hgb_cycle = HistGradientBoostingRegressor(max_iter=100, random_state=42)
    hgb_cycle.fit(X_train, y_cyc_tr)
    hgb_pred_cyc = hgb_cycle.predict(X_test)
    hgb_r2 = r2_score(y_cyc_te, hgb_pred_cyc)
    hgb_mae = mean_absolute_error(y_cyc_te, hgb_pred_cyc)

    print("\n" + "=" * 70)
    print("FINAL MODEL EVALUATION PERFORMANCE")
    print("=" * 70)
    print(f"PyTorch TrafficSignalResNet:")
    print(f"  * Cycle Length R^2:      {r2_cyc:.4f}  (MAE: {mae_cyc:.2f}s)")
    print(f"  * Intersection Delay R^2: {r2_del:.4f}  (MAE: {mae_del:.2f}s)")
    print(f"  * Route Travel Time R^2:  {r2_rou:.4f}  (MAE: {mae_rou:.2f}s)")
    print(f"\nGradient Boosted Trees (Benchmark):")
    print(f"  * Cycle Length R^2:      {hgb_r2:.4f}  (MAE: {hgb_mae:.2f}s)")
    print("=" * 70)
    print("TRAINING & BENCHMARKING COMPLETE!")

if __name__ == '__main__':
    train_pipeline()
