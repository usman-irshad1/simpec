"""
Inference & Predictive Routing Engine.
Demonstrates:
1. Predicting optimal cycle lengths & green splits using the trained TrafficSignalResNet.
2. Dynamic Route Cost evaluation & selecting better routes for traffic.
"""

import os
import joblib
import numpy as np
import torch
import torch.nn.functional as F

from models import TrafficSignalResNet

class MLTrafficController:
    def __init__(self, model_path=None, scaler_path=None):
        base_dir = os.path.dirname(os.path.abspath(__file__))
        if model_path is None:
            model_path = os.path.join(base_dir, 'traffic_signal_model.pth')
        if scaler_path is None:
            scaler_path = os.path.join(base_dir, 'scaler.pkl')

        self.device = torch.device('cpu')
        self.model = TrafficSignalResNet(input_dim=11, hidden_dim=128).to(self.device)
        self.model.load_state_dict(torch.load(model_path, map_location=self.device))
        self.model.eval()

        self.scaler = joblib.load(scaler_path)
        self.los_labels = ['LOS A (Free Flow)', 'LOS B (Stable)', 'LOS C (Light Delay)', 
                           'LOS D (Approaching Cap)', 'LOS E (Unstable)', 'LOS F (Gridlock)']

    def predict_signal_timing(self, vol_n, vol_s, vol_e, vol_w,
                              queue_n, queue_s, queue_e, queue_w,
                              hour=8.5, weather_grip=1.0, is_emergency=0):
        """
        Takes real-time intersection telemetry and predicts the optimal signal timing plan.
        """
        raw_feat = np.array([[
            vol_n, vol_s, vol_e, vol_w,
            queue_n, queue_s, queue_e, queue_w,
            hour, weather_grip, is_emergency
        ]])

        scaled_feat = self.scaler.transform(raw_feat)
        t_feat = torch.tensor(scaled_feat, dtype=torch.float32).to(self.device)

        with torch.no_grad():
            out = self.model(t_feat)
            cycle = out['cycle_length'].item()
            splits = out['splits'].cpu().numpy().flatten()
            delay = out['expected_delay'].item()
            los_idx = torch.argmax(out['los_logits'], dim=-1).item()
            route_tt = out['route_travel_time'].item()

        return {
            'optimal_cycle_sec': round(cycle, 1),
            'green_north_sec': round(splits[0], 1),
            'green_south_sec': round(splits[1], 1),
            'green_east_sec': round(splits[2], 1),
            'green_west_sec': round(splits[3], 1),
            'predicted_delay_sec': round(delay, 1),
            'predicted_los': self.los_labels[los_idx],
            'estimated_corridor_travel_time_sec': round(route_tt, 1)
        }

    def choose_best_route(self, origin, destination, candidate_routes):
        """
        Evaluates candidate corridors using ML predicted delay & dynamic congestion,
        selecting the globally superior minimum-delay path.
        """
        scored_routes = []
        for name, params in candidate_routes.items():
            pred = self.predict_signal_timing(
                params['vol_n'], params['vol_s'], params['vol_e'], params['vol_w'],
                params['queue_n'], params['queue_s'], params['queue_e'], params['queue_w'],
                hour=params.get('hour', 8.5),
                weather_grip=params.get('weather_grip', 1.0)
            )
            # Total Route Cost = Free Flow Time + Signal Delay + Queue Friction
            total_time = params['free_flow_time'] + pred['predicted_delay_sec']
            scored_routes.append({
                'route_name': name,
                'path': params['path'],
                'total_travel_time_sec': round(total_time, 1),
                'signal_delay_sec': pred['predicted_delay_sec'],
                'los': pred['predicted_los'],
                'optimal_signal_plan': pred
            })

        scored_routes.sort(key=lambda r: r['total_travel_time_sec'])
        return scored_routes

if __name__ == '__main__':
    controller = MLTrafficController()

    print("=" * 70)
    print("AI TRAFFIC SIGNAL OPTIMIZER & ROUTE ASSIGNMENT INFERENCE DEMO")
    print("=" * 70)

    # 1. Test Peak Rush Influx on Mount Olympus / Downtown Intersection
    print("\n--- SCENARIO 1: AM Peak Commute Rush (High North/South Volume) ---")
    plan = controller.predict_signal_timing(
        vol_n=1250, vol_s=1100, vol_e=320, vol_w=280,
        queue_n=16, queue_s=14, queue_e=3, queue_w=2,
        hour=8.5, weather_grip=1.0, is_emergency=0
    )
    print(f"  * Recommended Cycle Length: {plan['optimal_cycle_sec']}s")
    print(f"  * Green Splits: North={plan['green_north_sec']}s, South={plan['green_south_sec']}s, East={plan['green_east_sec']}s, West={plan['green_west_sec']}s")
    print(f"  * Total Green Duration:     {round(plan['green_north_sec'] + plan['green_south_sec'] + plan['green_east_sec'] + plan['green_west_sec'], 1)}s")
    print(f"  * Forecast Delay:           {plan['predicted_delay_sec']}s ({plan['predicted_los']})")

    # 2. Test Dynamic Route Selection between 2 alternative arterial corridors
    print("\n--- SCENARIO 2: Building Better Routes for Traffic (Route Comparison) ---")
    candidate_paths = {
        'Direct Main Boulevard (Downtown)': {
            'path': 'Origin -> Downtown Central -> Destination',
            'free_flow_time': 90.0,
            'vol_n': 1400, 'vol_s': 1200, 'vol_e': 400, 'vol_w': 350,
            'queue_n': 22, 'queue_s': 19, 'queue_e': 5, 'queue_w': 4
        },
        'Outer Arterial Bypass (Tech Ring Road)': {
            'path': 'Origin -> Tech Park Bypass -> Destination',
            'free_flow_time': 110.0, # longer physical distance
            'vol_n': 450, 'vol_s': 400, 'vol_e': 300, 'vol_w': 280,
            'queue_n': 3, 'queue_s': 2, 'queue_e': 2, 'queue_w': 1
        }
    }

    scored = controller.choose_best_route('Residential Sector', 'Commercial Core', candidate_paths)
    for rank, r in enumerate(scored, 1):
        status = "[RECOMMENDED OPTIMAL ROUTE]" if rank == 1 else "[CONGESTED - AVOID]"
        print(f"  Rank #{rank}: {r['route_name']} {status}")
        print(f"    Path:            {r['path']}")
        print(f"    Total Time:      {r['total_travel_time_sec']}s")
        print(f"    Signal Delay:    {r['signal_delay_sec']}s ({r['los']})")

    savings = scored[1]['total_travel_time_sec'] - scored[0]['total_travel_time_sec']
    print(f"\n>> AI ROUTING DECISION: Routing vehicles via {scored[0]['route_name']} saves {round(savings, 1)}s per trip!")
    print("=" * 70)
