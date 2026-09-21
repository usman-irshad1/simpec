"""
State-of-the-Art Deep Learning Architecture for Adaptive Traffic Signals & Predictive Routing:
Multi-Task Deep Residual Network (TrafficSignalResNet) with Simplex Split Constraint Projection.
"""

import torch
import torch.nn as nn
import torch.nn.functional as F

class ResBlock(nn.Module):
    def __init__(self, dim, dropout=0.08):
        super(ResBlock, self).__init__()
        self.fc1 = nn.Linear(dim, dim)
        self.norm1 = nn.LayerNorm(dim)
        self.act1 = nn.GELU()
        self.fc2 = nn.Linear(dim, dim)
        self.norm2 = nn.LayerNorm(dim)
        self.act2 = nn.GELU()
        self.drop = nn.Dropout(dropout)

    def forward(self, x):
        residual = x
        out = self.act1(self.norm1(self.fc1(x)))
        out = self.drop(out)
        out = self.norm2(self.fc2(out))
        out = self.act2(out + residual)
        return out

class TrafficSignalResNet(nn.Module):
    """
    Multi-Task Deep Residual Network for Traffic Signal Optimization & Route Assignment.
    Guarantees:
    1. Cycle Length is bounded in physical range [45s, 150s].
    2. Phase splits sum to exactly the effective green time (Cycle - LostTime).
    3. Concurrently predicts expected delay and HCM Level of Service (LOS).
    """
    def __init__(self, input_dim=11, hidden_dim=128, lost_time=16.0):
        super(TrafficSignalResNet, self).__init__()
        self.lost_time = lost_time

        # Input projection
        self.input_layer = nn.Sequential(
            nn.Linear(input_dim, hidden_dim),
            nn.LayerNorm(hidden_dim),
            nn.GELU()
        )

        # Deep Residual Backbone
        self.res1 = ResBlock(hidden_dim)
        self.res2 = ResBlock(hidden_dim)
        self.res3 = ResBlock(hidden_dim)

        # Head 1: Cycle Length (bounded [45s, 150s])
        self.cycle_head = nn.Sequential(
            nn.Linear(hidden_dim, 64),
            nn.GELU(),
            nn.Linear(64, 1)
        )

        # Head 2: Phase Split Logits (North, South, East, West)
        self.split_head = nn.Sequential(
            nn.Linear(hidden_dim, 64),
            nn.GELU(),
            nn.Linear(64, 4)
        )

        # Head 3: Expected Delay (seconds/veh)
        self.delay_head = nn.Sequential(
            nn.Linear(hidden_dim, 64),
            nn.GELU(),
            nn.Linear(64, 1)
        )

        # Head 4: HCM Level of Service (6 classes: LOS A to F)
        self.los_head = nn.Sequential(
            nn.Linear(hidden_dim, 64),
            nn.GELU(),
            nn.Linear(64, 6)
        )

        # Head 5: Route Dynamic Travel Time (seconds)
        self.route_head = nn.Sequential(
            nn.Linear(hidden_dim, 64),
            nn.GELU(),
            nn.Linear(64, 1)
        )

    def forward(self, x):
        features = self.input_layer(x)
        features = self.res1(features)
        features = self.res2(features)
        features = self.res3(features)

        # 1. Cycle Length C0: bounded [45s, 150s] using sigmoid scaling
        cycle_raw = self.cycle_head(features)
        cycle_len = 45.0 + torch.sigmoid(cycle_raw) * (150.0 - 45.0)

        # 2. Phase Splits with Simplex Projection (sum to C0 - lost_time)
        split_logits = self.split_head(features)
        split_ratios = F.softmax(split_logits, dim=-1)
        g_eff = torch.clamp(cycle_len - self.lost_time, min=24.0)
        splits = split_ratios * g_eff # Guaranteed sum = g_eff!

        # 3. Expected Delay (s/veh)
        delay = F.softplus(self.delay_head(features)) + 5.0

        # 4. HCM LOS Logits (A to F)
        los_logits = self.los_head(features)

        # 5. Route Travel Time (s)
        route_tt = F.softplus(self.route_head(features)) + 50.0

        return {
            'cycle_length': cycle_len,
            'splits': splits,
            'split_ratios': split_ratios,
            'expected_delay': delay,
            'los_logits': los_logits,
            'route_travel_time': route_tt
        }
