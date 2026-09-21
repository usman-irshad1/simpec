"""
Dataset Generator for Traffic Signal Optimization & Route Assignment.
Synthesizes comprehensive traffic operational data across diverse real-world regimes:
- AM Peak Commute (heavy asymmetric inbound flows)
- PM Peak Commute (heavy outbound arterial flows)
- Off-Peak / Midday (balanced moderate flows)
- Adverse Weather (Monsoon slick roads, Smog/Dense Fog with reduced grip)
- Road Bottlenecks / Incident Spills
"""

import os
import numpy as np
import pandas as pd

def generate_traffic_dataset(n_samples=5000, random_seed=42):
    np.random.seed(random_seed)
    
    # 1. Approach Volumes (veh/h) across 4 approaches: North, South, East, West
    # Simulating diverse regimes: Peak, Off-Peak, Bottleneck
    regimes = np.random.choice(['AM_PEAK', 'PM_PEAK', 'OFF_PEAK', 'WEEKEND', 'INCIDENT'], 
                               size=n_samples, p=[0.30, 0.30, 0.20, 0.10, 0.10])
    
    vol_n, vol_s, vol_e, vol_w = [], [], [], []
    hour_of_day = []
    weather_grip = []
    is_emergency = []
    
    for r in regimes:
        if r == 'AM_PEAK':
            h = np.random.uniform(7.0, 9.5)
            # Inbound heavy (North/South major commute corridors)
            vn = np.random.uniform(700, 1400)
            vs = np.random.uniform(500, 1100)
            ve = np.random.uniform(200, 500)
            vw = np.random.uniform(200, 450)
        elif r == 'PM_PEAK':
            h = np.random.uniform(16.5, 19.5)
            # Outbound heavy (East/West dispersion)
            vn = np.random.uniform(400, 800)
            vs = np.random.uniform(350, 750)
            ve = np.random.uniform(650, 1300)
            vw = np.random.uniform(600, 1200)
        elif r == 'OFF_PEAK':
            h = np.random.uniform(10.0, 16.0)
            vn = np.random.uniform(250, 550)
            vs = np.random.uniform(250, 550)
            ve = np.random.uniform(200, 450)
            vw = np.random.uniform(200, 450)
        elif r == 'WEEKEND':
            h = np.random.uniform(11.0, 21.0)
            vn = np.random.uniform(300, 700)
            vs = np.random.uniform(300, 700)
            ve = np.random.uniform(300, 700)
            vw = np.random.uniform(300, 700)
        else: # INCIDENT
            h = np.random.uniform(8.0, 18.0)
            vn = np.random.uniform(800, 1500)
            vs = np.random.uniform(200, 400) # bottlenecked
            ve = np.random.uniform(400, 900)
            vw = np.random.uniform(300, 700)
            
        vol_n.append(vn)
        vol_s.append(vs)
        vol_e.append(ve)
        vol_w.append(vw)
        hour_of_day.append(h)
        
        # Weather Grip (1.0 = Clear, 0.72 = Rain, 0.85 = Fog/Smog)
        w_roll = np.random.rand()
        if w_roll < 0.70:
            grip = 1.0
        elif w_roll < 0.85:
            grip = 0.72 # Monsoon slick
        else:
            grip = 0.85 # Smog / Dense fog
        weather_grip.append(grip)
        
        # Emergency preemption active
        is_emergency.append(1 if np.random.rand() < 0.04 else 0)
        
    vol_n = np.array(vol_n)
    vol_s = np.array(vol_s)
    vol_e = np.array(vol_e)
    vol_w = np.array(vol_w)
    hour_of_day = np.array(hour_of_day)
    weather_grip = np.array(weather_grip)
    is_emergency = np.array(is_emergency)
    
    # 2. Queue Depths (correlated with volume and weather)
    queue_n = np.maximum(0, (vol_n / 100.0) * np.random.uniform(0.8, 1.6, n_samples) * (1.1 / weather_grip))
    queue_s = np.maximum(0, (vol_s / 100.0) * np.random.uniform(0.8, 1.6, n_samples) * (1.1 / weather_grip))
    queue_e = np.maximum(0, (vol_e / 100.0) * np.random.uniform(0.8, 1.6, n_samples) * (1.1 / weather_grip))
    queue_w = np.maximum(0, (vol_w / 100.0) * np.random.uniform(0.8, 1.6, n_samples) * (1.1 / weather_grip))
    
    # 3. Approach Capacities and Saturation Flows
    sat_flow_base = 1900.0 # veh/h/lane
    sat_flow = sat_flow_base * weather_grip
    
    lost_time = 16.0 # 4 phases * 4s lost time per phase
    
    # 4. Webster & Genetic Algorithm Optimization Ground Truth
    # y_i = q_i / s_i
    y_n = vol_n / sat_flow
    y_s = vol_s / sat_flow
    y_e = vol_e / sat_flow
    y_w = vol_w / sat_flow
    
    Y = np.clip(y_n + y_s + y_e + y_w, 0.15, 0.88)
    
    # Webster optimal cycle length: C0 = (1.5 * L + 5) / (1 - Y)
    raw_c0 = (1.5 * lost_time + 5.0) / (1.0 - Y)
    
    # Genetic Algorithm fine-tuning perturbation (penalizing queue spillover)
    queue_imbalance = np.abs(queue_n - queue_s) + np.abs(queue_e - queue_w)
    ga_adjustment = (queue_imbalance / 20.0) * np.random.uniform(-3.0, 5.0, n_samples)
    
    opt_cycle_length = np.clip(raw_c0 + ga_adjustment, 45.0, 145.0)
    
    # Effective green time: g_eff = C0 - L
    g_eff = opt_cycle_length - lost_time
    
    # Green phase splits allocated proportionally with minimum green floor (6s)
    ratio_n = np.maximum(0.08, y_n / Y)
    ratio_s = np.maximum(0.08, y_s / Y)
    ratio_e = np.maximum(0.08, y_e / Y)
    ratio_w = np.maximum(0.08, y_w / Y)
    total_ratio = ratio_n + ratio_s + ratio_e + ratio_w
    
    split_n = np.maximum(6.0, (ratio_n / total_ratio) * g_eff)
    split_s = np.maximum(6.0, (ratio_s / total_ratio) * g_eff)
    split_e = np.maximum(6.0, (ratio_e / total_ratio) * g_eff)
    split_w = np.maximum(6.0, (ratio_w / total_ratio) * g_eff)
    
    # Adjust to exactly match g_eff
    sum_splits = split_n + split_s + split_e + split_w
    split_n = split_n * (g_eff / sum_splits)
    split_s = split_s * (g_eff / sum_splits)
    split_e = split_e * (g_eff / sum_splits)
    split_w = split_w * (g_eff / sum_splits)
    
    # 5. Resulting Intersection Delay (HCM 2016 control delay model)
    # d = 0.5 * C * (1 - g/C)^2 / (1 - min(1, X)*g/C) + 900T * [ (X-1) + sqrt((X-1)^2 + 8kIX/(cT)) ]
    mean_g_c = (g_eff / 4.0) / opt_cycle_length
    v_c_ratio = Y / np.clip(mean_g_c * 4.0, 0.2, 0.95)
    
    base_delay = 0.5 * opt_cycle_length * (1.0 - mean_g_c)**2 / np.maximum(0.05, 1.0 - mean_g_c * np.minimum(1.0, v_c_ratio))
    overflow_delay = 16.0 * np.maximum(0.0, v_c_ratio - 0.75)**1.8
    delay_sec = np.clip(base_delay + overflow_delay + np.random.normal(0, 1.5, n_samples), 6.0, 110.0)
    
    # HCM Level of Service (0=A, 1=B, 2=C, 3=D, 4=E, 5=F)
    los_grade = np.zeros(n_samples, dtype=int)
    los_grade[delay_sec > 10.0] = 1 # B
    los_grade[delay_sec > 20.0] = 2 # C
    los_grade[delay_sec > 35.0] = 3 # D
    los_grade[delay_sec > 55.0] = 4 # E
    los_grade[delay_sec > 80.0] = 5 # F
    
    # 6. Route Travel Time along arterial corridor (in seconds)
    free_flow_sec = 60.0 # 1 km at 60 km/h
    bpr_travel_time = free_flow_sec * (1.0 + 0.15 * (np.maximum(vol_n, vol_s) / 1200.0)**4.0) + (delay_sec * 0.4)
    route_travel_time = np.clip(bpr_travel_time, 65.0, 280.0)
    
    df = pd.DataFrame({
        'volume_north': vol_n,
        'volume_south': vol_s,
        'volume_east': vol_e,
        'volume_west': vol_w,
        'queue_north': queue_n,
        'queue_south': queue_s,
        'queue_east': queue_e,
        'queue_west': queue_w,
        'hour_of_day': hour_of_day,
        'weather_grip': weather_grip,
        'is_emergency': is_emergency,
        'saturation_flow': sat_flow,
        'target_cycle_length': opt_cycle_length,
        'target_split_north': split_n,
        'target_split_south': split_s,
        'target_split_east': split_e,
        'target_split_west': split_w,
        'target_delay_sec': delay_sec,
        'target_los_grade': los_grade,
        'target_route_travel_time': route_travel_time
    })
    
    return df

if __name__ == '__main__':
    out_dir = os.path.dirname(os.path.abspath(__file__))
    df = generate_traffic_dataset(n_samples=6000)
    csv_path = os.path.join(out_dir, 'traffic_dataset.csv')
    df.to_csv(csv_path, index=False)
    print(f"Generated {len(df)} operational samples saved to {csv_path}")
    print(df.head(3))
